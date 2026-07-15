#include "services/OpportunityService.h"
#include "store/InMemoryStore.h"
#include "models/Opportunity.h"
#include "Server.h"
#include "Session.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

OpportunityService::OpportunityService(InMemoryStore& store) : store_(store) {}

namespace {
    OpportunityType stringToOppType(const std::string& s)
    {
        if (s == "COMPETITION") return OpportunityType::COMPETITION;
        if (s == "JOB")         return OpportunityType::JOB;
        return OpportunityType::CLUB;
    }
    std::string oppTypeToString(OpportunityType t)
    {
        switch (t) {
        case OpportunityType::COMPETITION: return "COMPETITION";
        case OpportunityType::JOB:          return "JOB";
        default:                            return "CLUB";
        }
    }
}

//post an opportunity
void OpportunityService::handlePost(const Message& msg, std::shared_ptr<Session> sender)
{
    if (msg.title.empty() || msg.organization.empty())
    { sendError("title and organization are required", sender); return; }

    Opportunity opp;
    opp.opportunityId  = generateId();
    opp.posterUserId   = sender->userId();
    opp.posterUsername = msg.sender.username;
    opp.type           = stringToOppType(msg.oppType);
    opp.title          = msg.title;
    opp.description    = msg.text;
    opp.organization   = msg.organization;
    opp.deadline       = msg.deadline;
    opp.contactInfo    = msg.contactInfo;
    opp.status         = OpportunityStatus::OPEN;
    opp.createdAt      = currentTimestamp();

    if (!store_.addOpportunity(opp))
    { sendError("Failed to create opportunity", sender); return; }

    std::cout << "Opportunity created: " << opp.title << " by " << opp.posterUsername << "\n";

    Message resp;
    resp.type         = MessageType::OPP_POST;
    resp.parentId     = opp.opportunityId;
    resp.title        = opp.title;
    resp.text         = opp.description;
    resp.organization = opp.organization;
    resp.deadline     = opp.deadline;
    resp.contactInfo  = opp.contactInfo;
    resp.oppType      = oppTypeToString(opp.type);
    resp.sender.username = opp.posterUsername;
    resp.sender.userId   = opp.posterUserId;
    sender->send(resp);

    if (server_) server_->broadcast(resp, sender);
}

//delete own opportunity
void OpportunityService::handleDelete(const Message& msg, std::shared_ptr<Session> sender)
{
    if (msg.parentId.empty())
    { sendError("opportunityId (parentId) is required", sender); return; }

    if (!store_.deleteOpportunity(msg.parentId, sender->userId()))
    { sendError("Opportunity not found or you are not the poster", sender); return; }

    Message resp;
    resp.type     = MessageType::OPP_DELETE;
    resp.parentId = msg.parentId;
    sender->send(resp);
    if (server_) server_->broadcast(resp, sender);
}

//search opportunities
void OpportunityService::handleSearch(const Message& msg, std::shared_ptr<Session> sender)
{
    // msg.text is the search query — empty = return all open opportunities
    auto opportunities = store_.searchOpportunities(msg.text);

    for (auto& o : opportunities)
    {
        Message resp;
        resp.type         = MessageType::OPP_POST;
        resp.parentId     = o.opportunityId;
        resp.title        = o.title;
        resp.text         = o.description;
        resp.organization = o.organization;
        resp.deadline     = o.deadline;
        resp.contactInfo  = o.contactInfo;
        resp.oppType      = oppTypeToString(o.type);
        resp.sender.userId   = o.posterUserId;
        resp.sender.username = o.posterUsername;
        resp.timestamp    = o.createdAt;
        sender->send(resp);
    }

    if (opportunities.empty())
        sendOk("No opportunities found", sender);
}

//apply / contact poster
void OpportunityService::handleApply(const Message& msg, std::shared_ptr<Session> sender)
{
    if (msg.parentId.empty())
    { sendError("opportunityId (parentId) is required", sender); return; }

    auto oppOpt = store_.findOpportunity(msg.parentId);
    if (!oppOpt)
    { sendError("Opportunity not found", sender); return; }

    if (oppOpt->status != OpportunityStatus::OPEN)
    { sendError("This opportunity is no longer open", sender); return; }

    if (oppOpt->posterUserId == sender->userId())
    { sendError("You cannot apply to your own opportunity", sender); return; }

    //notify poster
    Message notif;
    notif.type = MessageType::OPP_APPLY;
    notif.parentId = oppOpt->opportunityId;
    notif.sender.userId   = sender->userId();
    notif.sender.username = msg.sender.username;
    notif.title = oppOpt->title;
    notif.text = msg.text.empty()
        ? msg.sender.username + " is interested in your opportunity: " + oppOpt->title
        : msg.text;
    notif.timestamp = currentTimestamp();

    if (server_) server_->sendTo(oppOpt->posterUserId, notif);

    Message confirm;
    confirm.type = MessageType::OPP_APPLY;
    confirm.text = "Application sent to poster.";
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
