#pragma once
#include <memory>
#include "Message.h"

class Session;
class InMemoryStore;

class ForumService {
public:
    explicit ForumService(InMemoryStore& store);

    void handleQuestion(const Message& msg, std::shared_ptr<Session> sender); // post question
    void handleAnswer  (const Message& msg, std::shared_ptr<Session> sender); // post answer
    void handleFaq     (const Message& msg, std::shared_ptr<Session> sender); // mark answer as FAQ
    void handleGetAll  (const Message& msg, std::shared_ptr<Session> sender); // list all questions
    void handleGetOne  (const Message& msg, std::shared_ptr<Session> sender); // get question + answers
    void handleVote    (const Message& msg, std::shared_ptr<Session> sender);

private:
    std::string generateId();
    std::string currentTimestamp();
    void sendError(const std::string& reason, std::shared_ptr<Session> sender);
    void sendOk   (const std::string& text,   std::shared_ptr<Session> sender);

    InMemoryStore& store_;
};