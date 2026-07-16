#pragma once
#include <memory>
#include "Message.h"

class Session;
class InMemoryStore;
class Server; 

class ProfileService {
public:
    explicit ProfileService(InMemoryStore& store);
    void setServer(Server& server) { server_ = &server; } 

    void handleGet (const Message& msg, std::shared_ptr<Session> sender);
    void handleEdit(const Message& msg, std::shared_ptr<Session> sender);
    void handleView(const Message& msg, std::shared_ptr<Session> sender); 

private:
    std::string hashPassword(const std::string& plain);
    void sendError(const std::string& reason, std::shared_ptr<Session> sender);
    InMemoryStore& store_;
    Server* server_ = nullptr;
};