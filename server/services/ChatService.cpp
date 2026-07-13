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

//give access to database
ChatService::ChatService(InMemoryStore& store) : store_(store){}

//creates a new room
void ChatService::handleCreate(const Message& msg, std::shared_ptr<Session> sender)
{
    if (msg.roomId.empty() || msg.text.empty())
    { sendError("roomId and room name (text) are required", sender); return; }

    ChatRoom room;
    room.roomId = msg.roomId;
    room.name = msg.text;
    room.creatorId = sender->userId();
    room.createdAt = currentTimestamp();

    if (msg.role == "GROUP")  room.type = RoomType::GROUP;
    else if (msg.role == "DIRECT") room.type = RoomType::DIRECT;
    else room.type = RoomType::PUBLIC;

    // creator is always a member
    room.memberIds.push_back(sender->userId());

    // for private/group rooms, add the invited members (comma-separated userIds in mediaUrl)
    if (room.type == RoomType::GROUP && !msg.mediaUrl.empty())
    {
        std::stringstream ss(msg.mediaUrl);
        std::string memberId;
        while (std::getline(ss, memberId, ','))
        {
            if (!memberId.empty() && memberId != sender->userId())
                room.memberIds.push_back(memberId);
        }
    }

    if (!store_.createRoom(room))
    { sendError("Room ID already exists", sender); return; }

    std::cout << "Room created: " << room.roomId << " (" << msg.role << ")\n";

    // pack the member list so clients can populate the room's right-side member panel
    std::string memberList;
    for (size_t i = 0; i < room.memberIds.size(); ++i) {
        memberList += room.memberIds[i];
        if (i + 1 < room.memberIds.size()) memberList += ",";
    }

    Message notif;
    notif.type     = MessageType::CHAT_CREATE;
    notif.roomId   = room.roomId;
    notif.text     = room.name;
    notif.role     = msg.role;
    notif.mediaUrl = memberList;
    notif.sender.userId   = sender->userId();
    notif.sender.username = msg.sender.username;

    if (server_)
    {
        if (room.type == RoomType::GROUP)
        {
            // private room — only the invited members get notified, not everyone
            for (const auto& memberId : room.memberIds)
                if (memberId != sender->userId())
                    server_->sendTo(memberId, notif);
        }
        else
        {
            // public room — announce to everyone
            server_->broadcast(notif, sender);
        }
    }

    sendOk("Room '" + room.name + "' created", sender);
}

//join a room
void ChatService::handleJoin(const Message& msg, std::shared_ptr<Session> sender)
{
    //input validation
    if (msg.roomId.empty())
    { sendError("roomId is required", sender); return; }

    auto roomOpt = store_.findRoom(msg.roomId);
    if (!roomOpt)
    { sendError("Room not found", sender); return; }

    //DIRECT rooms are 1-on-1 so you can't freely join
    if (roomOpt->type == RoomType::DIRECT)
    { sendError("Cannot join a direct message room", sender); return; }

    store_.addMemberToRoom(msg.roomId, sender->userId());   //add user to room

    //notify room that someone joined
    Message notif;
    notif.type = MessageType::JOIN;
    notif.roomId = msg.roomId;
    notif.sender.userId = sender->userId();
    notif.sender.username = msg.sender.username;
    notif.text = msg.sender.username + " joined the room";
    notif.timestamp = currentTimestamp();
    if(server_) server_->sendToRoom(msg.roomId, notif, nullptr);

    sendOk("Joined room " + msg.roomId, sender);    //tell client that connection was successfull
}

//leave a room NOTE: this doesnt make the user actually leave, it only notifies the room, this will be implemented later on
void ChatService::handleLeave(const Message& msg, std::shared_ptr<Session> sender)
{
    if (msg.roomId.empty())
    { sendError("roomId is required", sender); return; }

    //notify room that someone left
    Message notif;
    notif.type = MessageType::LEAVE;
    notif.roomId = msg.roomId;
    notif.sender.userId = sender->userId();
    notif.sender.username = msg.sender.username;
    notif.text = msg.sender.username + " left the room";
    notif.timestamp = currentTimestamp();
    if(server_) server_->sendToRoom(msg.roomId, notif, sender);

    sendOk("Left room " + msg.roomId, sender);  //tell client that the action was successfull
}

//send a message to a room
void ChatService::handlePublic(const Message& msg, std::shared_ptr<Session> sender)
{
    if (msg.roomId.empty() || msg.text.empty())
    { sendError("roomId and text are required", sender); return; }

    auto roomOpt = store_.findRoom(msg.roomId);
    if (!roomOpt)
    { sendError("Room not found", sender); return; }

    if (!store_.isMember(msg.roomId, sender->userId()))
    { sendError("You are not a member of this room", sender); return; }

    //stamp timestamp server side
    Message out    = msg;
    out.timestamp  = currentTimestamp();
    out.sender.userId = sender->userId();

    //create message and save to history
    ChatMessage cm;
    cm.messageId = generateId();
    cm.roomId = msg.roomId;
    cm.senderUserId = sender->userId();
    cm.senderUsername = msg.sender.username;
    cm.text = msg.text;
    cm.mediaUrl = msg.mediaUrl;
    cm.timestamp = out.timestamp;
    store_.addMessageToRoom(msg.roomId, cm);

    //broadcast to room, every member of room recieves the message
    if(server_) server_->sendToRoom(msg.roomId, out, nullptr);
}

//sends a direct 1-1 message
void ChatService::handlePrivate(const Message& msg, std::shared_ptr<Session> sender)
{
    if (msg.recipientId.empty() || msg.text.empty())
    { sendError("recipientId and text are required", sender); return; }

    //build or find the direct room between these two users
    //roomID is direct + sorted userIds joined by ':' to prevent 2 different direct message rooms between the same 2 from happening ("Alice:Bob, Bob:Alice")
    std::string uid1 = sender->userId();
    std::string uid2 = msg.recipientId;
    if (uid1 > uid2) std::swap(uid1, uid2);
    std::string directRoomId = "direct:" + uid1 + ":" + uid2;

    //create the direct room if it doesn't exist
    if (!store_.findRoom(directRoomId))
    {
        ChatRoom room;
        room.roomId = directRoomId;
        room.name = "Direct";
        room.type = RoomType::DIRECT;
        room.creatorId = sender->userId();
        room.memberIds = { uid1, uid2 };
        room.createdAt = currentTimestamp();
        store_.createRoom(room);
    }

    //save message
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

    //send to recipient and echo back to sender to dispay the message on both ends
    if(server_) server_->sendTo(msg.recipientId, out);
    sender->send(out);
}

//returns all previous messages from a room
void ChatService::handleHistory(const Message& msg, std::shared_ptr<Session> sender)
{
    if (msg.roomId.empty())
    { sendError("roomId is required", sender); return; }

    if (!store_.isMember(msg.roomId, sender->userId()))
    { sendError("You are not a member of this room", sender); return; }

    auto history = store_.getRoomHistory(msg.roomId);   //retrieve room history
    for (auto& cm : history)    //loop and send to client (the client rebuilds the chat window from these messages)
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

//generates a random message ID
std::string ChatService::generateId()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    std::ostringstream oss;
    oss << std::hex << dis(gen);
    return oss.str();
}

//gets current time
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
    Message m; m.type = MessageType::PRESENCE; m.text = text;
    sender->send(m);
}