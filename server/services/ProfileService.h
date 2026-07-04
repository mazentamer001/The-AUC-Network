#pragma once
#include <memory>
#include "Message.h"

class Session;
class InMemoryStore;

class ProfileService {
public:
    explicit ProfileService(InMemoryStore& store);

    void handleGet (const Message& msg, std::shared_ptr<Session> sender);
    void handleEdit(const Message& msg, std::shared_ptr<Session> sender);

private:
    void sendError(const std::string& reason, std::shared_ptr<Session> sender);
    InMemoryStore& store_;
};