#include "services/OpportunityService.h"
#include "store/InMemoryStore.h"
#include "models/Opportunity.h"
#include "models/ChatRoom.h"
#include "Server.h"
#include "Session.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

OpportunityService::OpportunityService(InMemoryStore& store) : store_(store) {}

// post an opportunity
void OpportunityService::handlePost(const Message& msg, std::shared_ptr<Session> sender)
{
    if (msg.title.empty())
    { sendError("title is required", sender); return; }

    Opportunity opp;
    opp.opportunityId    = generateId();
    opp.posterUserId     = sender->userId();
    opp.posterUsername   = msg.sender.username;
    opp.title             = msg.title;
    opp.description       = msg.text;
    opp.category           = msg.category.empty() ? "Other" : msg.category;
    opp.location            = msg.location;
    opp.mediaUrl            = msg.mediaUrl;
    opp.status               = OpportunityStatus::ACTIVE;
    opp.createdAt           = currentTimestamp();

    if (!store_.addOpportunity(opp))
    { sendError("Failed to create opportunity", sender); return; }

    std::cout << "Opportunity created: " << opp.title << " by " << opp.posterUsername << "\n";

    Message resp;
    resp.type            = MessageType::OPP_POST;
    resp.parentId         = opp.opportunityId;
    resp.title             = opp.title;
    resp.text               = opp.description;
    resp.category             = opp.category;
    resp.location            = opp.location;
    resp.mediaUrl            = opp.mediaUrl;
    resp.sender.username    = opp.posterUsername;
    resp.sender.userId      = opp.posterUserId;
    resp.timestamp            = opp.createdAt;
    sender->send(resp);

    if (server_) server_->broadcast(resp, sender);
}

// delete own opportunity
void OpportunityService::handleDelete(const Message& msg, std::shared_ptr<Session> sender)
{
    if (msg.parentId.empty())
    { sendError("opportunityId (parentId) is required", sender); return; }

    if (!store_.deleteOpportunity(msg.parentId, sender->userId()))
    { sendError("Opportunity not found or you are not the poster", sender); return; }

    Message resp;
    resp.type     = MessageType::OPP_DELETE;
    resp.parentId  = msg.parentId;
    sender->send(resp);
    if (server_) server_->broadcast(resp, sender);
}

// search opportunities
void OpportunityService::handleSearch(const Message& msg, std::shared_ptr<Session> sender)
{
    // msg.text is the search query — empty = return all active opportunities
    auto opportunities = store_.searchOpportunities(msg.text);

    for (auto& o : opportunities)
    {
        Message resp;
        resp.type      = MessageType::OPP_POST;
        resp.parentId    = o.opportunityId;
        resp.title        = o.title;
        resp.text          = o.description;
        resp.category       = o.category;
        resp.location       = o.location;
        resp.mediaUrl       = o.mediaUrl;
        resp.sender.userId    = o.posterUserId;
        resp.sender.username  = o.posterUsername;
        resp.timestamp          = o.createdAt;
        sender->send(resp);
    }

    if (opportunities.empty())
        sendOk("No opportunities found", sender);
}

// inquiry / apply
// Opens a 1-on-1 direct chat room between applicant and poster, same
// deterministic room-id scheme used by MarketplaceService, then notifies
// the poster with the opportunity details.
void OpportunityService::handleInquiry(const Message& msg, std::shared_ptr<Session> sender)
{
    if (msg.parentId.empty())
    { sendError("opportunityId (parentId) is required", sender); return; }

    auto oppOpt = store_.findOpportunity(msg.parentId);
    if (!oppOpt)
    { sendError("Opportunity not found", sender); return; }

    if (oppOpt->status != OpportunityStatus::ACTIVE)
    { sendError("This opportunity is no longer available", sender); return; }

    if (oppOpt->posterUserId == sender->userId())
    { sendError("You cannot apply to your own opportunity", sender); return; }

    // build deterministic direct room ID between applicant and poster
    std::string uid1 = sender->userId();
    std::string uid2 = oppOpt->posterUserId;
    if (uid1 > uid2) std::swap(uid1, uid2);
    std::string directRoomId = "direct:" + uid1 + ":" + uid2;

    if (!store_.findRoom(directRoomId))
    {
        ChatRoom room;
        room.roomId    = directRoomId;
        room.name       = "Direct";
        room.type        = RoomType::DIRECT;
        room.creatorId    = sender->userId();
        room.memberIds     = { uid1, uid2 };
        room.createdAt       = currentTimestamp();
        store_.createRoom(room);
    }

    // notify poster via direct message
    Message notif;
    notif.type            = MessageType::OPP_INQUIRY;
    notif.roomId            = directRoomId;
    notif.sender.userId       = sender->userId();
    notif.sender.username      = msg.sender.username;
    notif.title                  = oppOpt->title;
    notif.text = msg.text.empty()
        ? msg.sender.username + " is interested in your opportunity: " + oppOpt->title
        : msg.text;
    notif.timestamp = currentTimestamp();

    if (server_) server_->sendTo(oppOpt->posterUserId, notif);

    // confirm to applicant with the room ID so they can follow up
    Message confirm;
    confirm.type    = MessageType::OPP_INQUIRY;
    confirm.text     = "Application sent to poster. Use roomId to continue the chat.";
    confirm.roomId    = directRoomId;
    sender->send(confirm);
}

std::string OpportunityService::generateId()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    std::ostringstream oss;
    oss << std::hex << dis(gen);
    return oss.str();
}

std::string OpportunityService::currentTimestamp()
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&t), "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

void OpportunityService::sendError(const std::string& reason, std::shared_ptr<Session> sender)
{
    Message m; m.type = MessageType::ERROR; m.text = reason;
    sender->send(m);
}

void OpportunityService::sendOk(const std::string& text, std::shared_ptr<Session> sender)
{
    Message m; m.type = MessageType::OPP_POST; m.text = text;
    sender->send(m);
}