#include "services/ChatService.h"
#include "store/InMemoryStore.h"
#include "models/ChatRoom.h"
#include "Server.h"
#include "Session.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

ChatService::ChatService(InMemoryStore& store)
    : store_(store)
{}

// ── create a room ─────────────────────────────────────────────────────────────
void ChatService::handleCreate(const Message& msg, std::shared_ptr<Session> sender)
{
    if (msg.roomId.empty() || msg.text.empty())
    { sendError("roomId and room name (text) are required", sender); return; }

    ChatRoom room;
    room.roomId    = msg.roomId;
    room.name      = msg.text;
    room.creatorId = sender->userId();
    room.createdAt = currentTimestamp();

    // type comes from msg.role field: "PUBLIC", "GROUP", "DIRECT"
    if      (msg.role == "GROUP")  room.type = RoomType::GROUP;
    else if (msg.role == "DIRECT") room.type = RoomType::DIRECT;
    else                           room.type = RoomType::PUBLIC;

    // Creator is always a member
    room.memberIds.push_back(sender->userId());

    if (!store_.createRoom(room))
    { sendError("Room ID already exists", sender); return; }

    std::cout << "Room created: " << room.roomId << " (" << msg.role << ")\n";
    sendOk("Room '" + room.name + "' created", sender);
}

// ── join a room ───────────────────────────────────────────────────────────────
void ChatService::handleJoin(const Message& msg, std::shared_ptr<Session> sender)
{
    if (msg.roomId.empty())
    { sendError("roomId is required", sender); return; }

    auto roomOpt = store_.findRoom(msg.roomId);
    if (!roomOpt)
    { sendError("Room not found", sender); return; }

    // DIRECT rooms are 1-on-1 — can't freely join
    if (roomOpt->type == RoomType::DIRECT)
    { sendError("Cannot join a direct message room", sender); return; }

    store_.addMemberToRoom(msg.roomId, sender->userId());

    // Notify room that someone joined
    Message notif;
    notif.type            = MessageType::JOIN;
    notif.roomId          = msg.roomId;
    notif.sender.userId   = sender->userId();
    notif.sender.username = msg.sender.username;
    notif.text            = msg.sender.username + " joined the room";
    notif.timestamp       = currentTimestamp();
    if(server_) server_->sendToRoom(msg.roomId, notif, nullptr);

    sendOk("Joined room " + msg.roomId, sender);
}

// ── leave a room ──────────────────────────────────────────────────────────────
void ChatService::handleLeave(const Message& msg, std::shared_ptr<Session> sender)
{
    if (msg.roomId.empty())
    { sendError("roomId is required", sender); return; }

    Message notif;
    notif.type            = MessageType::LEAVE;
    notif.roomId          = msg.roomId;
    notif.sender.userId   = sender->userId();
    notif.sender.username = msg.sender.username;
    notif.text            = msg.sender.username + " left the room";
    notif.timestamp       = currentTimestamp();
    if(server_) server_->sendToRoom(msg.roomId, notif, sender);

    sendOk("Left room " + msg.roomId, sender);
}

// ── public message ────────────────────────────────────────────────────────────
void ChatService::handlePublic(const Message& msg, std::shared_ptr<Session> sender)
{
    if (msg.roomId.empty() || msg.text.empty())
    { sendError("roomId and text are required", sender); return; }

    auto roomOpt = store_.findRoom(msg.roomId);
    if (!roomOpt)
    { sendError("Room not found", sender); return; }

    if (!store_.isMember(msg.roomId, sender->userId()))
    { sendError("You are not a member of this room", sender); return; }

    // Stamp timestamp server-side
    Message out    = msg;
    out.timestamp  = currentTimestamp();
    out.sender.userId = sender->userId();

    // Persist to history
    ChatMessage cm;
    cm.messageId      = generateId();
    cm.roomId         = msg.roomId;
    cm.senderUserId   = sender->userId();
    cm.senderUsername = msg.sender.username;
    cm.text           = msg.text;
    cm.mediaUrl       = msg.mediaUrl;
    cm.timestamp      = out.timestamp;
    store_.addMessageToRoom(msg.roomId, cm);

    // Broadcast to room
    if(server_) server_->sendToRoom(msg.roomId, out, nullptr);
}

// ── private (direct) message ──────────────────────────────────────────────────
void ChatService::handlePrivate(const Message& msg, std::shared_ptr<Session> sender)
{
    if (msg.recipientId.empty() || msg.text.empty())
    { sendError("recipientId and text are required", sender); return; }

    // Build or find the direct room between these two users
    // Room ID is deterministic: sorted userIds joined by ':'
    std::string uid1 = sender->userId();
    std::string uid2 = msg.recipientId;
    if (uid1 > uid2) std::swap(uid1, uid2);
    std::string directRoomId = "direct:" + uid1 + ":" + uid2;

    // Auto-create the direct room if it doesn't exist
    if (!store_.findRoom(directRoomId))
    {
        ChatRoom room;
        room.roomId    = directRoomId;
        room.name      = "Direct";
        room.type      = RoomType::DIRECT;
        room.creatorId = sender->userId();
        room.memberIds = { uid1, uid2 };
        room.createdAt = currentTimestamp();
        store_.createRoom(room);
    }

    Message out       = msg;
    out.roomId        = directRoomId;
    out.timestamp     = currentTimestamp();
    out.sender.userId = sender->userId();

    ChatMessage cm;
    cm.messageId      = generateId();
    cm.roomId         = directRoomId;
    cm.senderUserId   = sender->userId();
    cm.senderUsername = msg.sender.username;
    cm.text           = msg.text;
    cm.timestamp      = out.timestamp;
    store_.addMessageToRoom(directRoomId, cm);

    // Send to recipient and echo back to sender
    if(server_) server_->sendTo(msg.recipientId, out);
    sender->send(out);
}

// ── get history ───────────────────────────────────────────────────────────────
void ChatService::handleHistory(const Message& msg, std::shared_ptr<Session> sender)
{
    if (msg.roomId.empty())
    { sendError("roomId is required", sender); return; }

    if (!store_.isMember(msg.roomId, sender->userId()))
    { sendError("You are not a member of this room", sender); return; }

    auto history = store_.getRoomHistory(msg.roomId);
    for (auto& cm : history)
    {
        Message out;
        out.type            = MessageType::CHAT_PUBLIC;
        out.roomId          = cm.roomId;
        out.sender.userId   = cm.senderUserId;
        out.sender.username = cm.senderUsername;
        out.text            = cm.text;
        out.mediaUrl        = cm.mediaUrl;
        out.timestamp       = cm.timestamp;
        sender->send(out);
    }
}

// ── helpers ───────────────────────────────────────────────────────────────────
std::string ChatService::generateId()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    std::ostringstream oss;
    oss << std::hex << dis(gen);
    return oss.str();
}

std::string ChatService::currentTimestamp()
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&t), "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

void ChatService::sendError(const std::string& reason, std::shared_ptr<Session> sender)
{
    Message m; m.type = MessageType::ERROR; m.text = reason;
    sender->send(m);
}

void ChatService::sendOk(const std::string& text, std::shared_ptr<Session> sender)
{
    Message m; m.type = MessageType::AUTH_RESPONSE; m.text = text;
    sender->send(m);
}