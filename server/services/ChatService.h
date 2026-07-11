#pragma once
#include <memory>
#include "Message.h"

class Session;
class Server;
class Database;

class ChatService {
public:
    ChatService(Database& store);
    void setServer(Server& server) { server_ = &server; }

    void handlePublic (const Message& msg, std::shared_ptr<Session> sender);
    void handlePrivate(const Message& msg, std::shared_ptr<Session> sender);
    void handleJoin   (const Message& msg, std::shared_ptr<Session> sender);
    void handleLeave  (const Message& msg, std::shared_ptr<Session> sender);
    void handleCreate (const Message& msg, std::shared_ptr<Session> sender);
    void handleHistory(const Message& msg, std::shared_ptr<Session> sender);

private:
    std::string generateId();
    std::string currentTimestamp();
    void sendError(const std::string& reason, std::shared_ptr<Session> sender);
    void sendOk   (const std::string& text,   std::shared_ptr<Session> sender);

    Server*server_ = nullptr;
    Database& store_;
};