#pragma once
#include <memory>
#include <string>
#include "Message.h"

class Session;
class Server;
class InMemoryStore;

// Mirrors MarketplaceService exactly: post / delete / search / inquiry.
// Inquiry opens the same deterministic direct-chat room pattern used by
// the marketplace, so "apply" / "express interest" reuses the chat system.
class OpportunityService {
public:
    OpportunityService(InMemoryStore& store);
    void setServer(Server& server) { server_ = &server; }

    void handlePost   (const Message& msg, std::shared_ptr<Session> sender); // create opportunity
    void handleDelete (const Message& msg, std::shared_ptr<Session> sender); // delete own opportunity
    void handleSearch (const Message& msg, std::shared_ptr<Session> sender); // search / list opportunities
    void handleInquiry(const Message& msg, std::shared_ptr<Session> sender); // apply / contact poster

private:
    std::string generateId();
    std::string currentTimestamp();
    void sendError(const std::string& reason, std::shared_ptr<Session> sender);
    void sendOk(const std::string& text, std::shared_ptr<Session> sender);

    Server* server_ = nullptr;
    InMemoryStore& store_;
};