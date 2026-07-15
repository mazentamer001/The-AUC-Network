#pragma once
#include <memory>
#include "Message.h"

class Session;
class Server;
class InMemoryStore;

class OpportunityService {
public:
    OpportunityService(InMemoryStore& store);
    void setServer(Server& server) { server_ = &server; }

    void handlePost   (const Message& msg, std::shared_ptr<Session> sender);
    void handleDelete (const Message& msg, std::shared_ptr<Session> sender);
    void handleSearch (const Message& msg, std::shared_ptr<Session> sender);
    void handleApply  (const Message& msg, std::shared_ptr<Session> sender);

private:
    std::string generateId();
    std::string currentTimestamp();
    void sendError(const std::string& reason, std::shared_ptr<Session> sender);
    void sendOk   (const std::string& text,   std::shared_ptr<Session> sender);

    Server*        server_ = nullptr;
    InMemoryStore& store_;
};
