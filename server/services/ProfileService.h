#pragma once
#include <memory>
#include "Message.h"

class Session;
class Database;

class ProfileService {
public:
    explicit ProfileService(Database& store);

    void handleGet (const Message& msg, std::shared_ptr<Session> sender);
    void handleEdit(const Message& msg, std::shared_ptr<Session> sender);

private:
    std::string hashPassword(const std::string& plain);
    void sendError(const std::string& reason, std::shared_ptr<Session> sender);
    Database& store_;
};