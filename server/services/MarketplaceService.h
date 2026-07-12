#pragma once
#include <memory>
#include "Message.h"

class Session;
class Server;
class InMemoryStore;

class MarketplaceService {
public:
    MarketplaceService(InMemoryStore& store);
    void setServer(Server& server) { server_ = &server; }

    void handlePost (const Message& msg, std::shared_ptr<Session> sender); // create listing
    void handleDelete (const Message& msg, std::shared_ptr<Session> sender); // delete own listing
    void handleSearch (const Message& msg, std::shared_ptr<Session> sender); // search listings
    void handleInquiry (const Message& msg, std::shared_ptr<Session> sender); // buy / contact seller

private:
    std::string generateId();
    std::string currentTimestamp();
    void sendError(const std::string& reason, std::shared_ptr<Session> sender);
    void sendOk (const std::string& text,   std::shared_ptr<Session> sender);

    Server* server_ = nullptr;
    InMemoryStore& store_;
};