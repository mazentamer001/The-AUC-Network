#include "services/ForumService.h"
#include "store/InMemoryStore.h"
#include "models/ForumPost.h"
#include "Session.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

ForumService::ForumService(InMemoryStore& store) : store_(store) {}

// ── post a question ───────────────────────────────────────────────────────────
void ForumService::handleQuestion(const Message& msg, std::shared_ptr<Session> sender)
{
    if (msg.title.empty() || msg.text.empty())
    { sendError("title and text are required", sender); return; }

    ForumQuestion q;
    q.questionId     = generateId();
    q.authorUserId   = sender->userId();
    q.authorUsername = msg.sender.username;
    q.title          = msg.title;
    q.text           = msg.text;
    q.timestamp      = currentTimestamp();

    if (!store_.addQuestion(q))
    { sendError("Failed to post question", sender); return; }

    std::cout << "Forum question posted: " << q.title << "\n";

    Message resp;
    resp.type     = MessageType::QA_QUESTION;
    resp.parentId = q.questionId;   // return questionId to client
    resp.text     = "Question posted: " + q.title;
    sender->send(resp);
}

// ── post an answer ────────────────────────────────────────────────────────────
// parentId = questionId being answered
void ForumService::handleAnswer(const Message& msg, std::shared_ptr<Session> sender)
{
    if (msg.parentId.empty() || msg.text.empty())
    { sendError("parentId (questionId) and text are required", sender); return; }

    auto qOpt = store_.findQuestion(msg.parentId);
    if (!qOpt)
    { sendError("Question not found", sender); return; }

    ForumAnswer answer;
    answer.answerId       = generateId();
    answer.questionId     = msg.parentId;
    answer.authorUserId   = sender->userId();
    answer.authorUsername = msg.sender.username;
    answer.text           = msg.text;
    answer.isFaq          = false;
    answer.timestamp      = currentTimestamp();

    if (!store_.addAnswer(msg.parentId, answer))
    { sendError("Failed to post answer", sender); return; }

    Message resp;
    resp.type     = MessageType::QA_ANSWER;
    resp.parentId = answer.answerId;
    resp.text     = "Answer posted";
    sender->send(resp);
}

// ── mark own answer as FAQ ────────────────────────────────────────────────────
// parentId = questionId, filename = answerId (reusing a spare field)
void ForumService::handleFaq(const Message& msg, std::shared_ptr<Session> sender)
{
    if (msg.parentId.empty() || msg.filename.empty())
    { sendError("parentId (questionId) and filename (answerId) are required", sender); return; }

    if (!store_.markAnswerFaq(msg.parentId, msg.filename, sender->userId()))
    { sendError("Answer not found or you are not the author", sender); return; }

    sendOk("Answer marked as FAQ", sender);
}

// ── get all questions (list view) ─────────────────────────────────────────────
void ForumService::handleGetAll(const Message& msg, std::shared_ptr<Session> sender)
{
    auto questions = store_.getAllQuestions();

    for (auto& q : questions)
    {
        Message resp;
        resp.type            = MessageType::QA_QUESTION;
        resp.parentId        = q.questionId;
        resp.title           = q.title;
        resp.text            = q.text;
        resp.sender.userId   = q.authorUserId;
        resp.sender.username = q.authorUsername;
        resp.timestamp       = q.timestamp;
        sender->send(resp);
    }

    if (questions.empty())
        sendOk("No questions yet", sender);
}

// ── get one question with all its answers ─────────────────────────────────────
void ForumService::handleGetOne(const Message& msg, std::shared_ptr<Session> sender)
{
    if (msg.parentId.empty())
    { sendError("parentId (questionId) is required", sender); return; }

    auto qOpt = store_.findQuestion(msg.parentId);
    if (!qOpt)
    { sendError("Question not found", sender); return; }

    // Send the question first
    Message qResp;
    qResp.type            = MessageType::QA_QUESTION;
    qResp.parentId        = qOpt->questionId;
    qResp.title           = qOpt->title;
    qResp.text            = qOpt->text;
    qResp.sender.userId   = qOpt->authorUserId;
    qResp.sender.username = qOpt->authorUsername;
    qResp.timestamp       = qOpt->timestamp;
    sender->send(qResp);

    // Then send each answer
    for (auto& a : qOpt->answers)
    {
        Message aResp;
        aResp.type            = MessageType::QA_ANSWER;
        aResp.parentId        = a.questionId;
        aResp.filename        = a.answerId;
        aResp.text            = a.text;
        aResp.sender.userId   = a.authorUserId;
        aResp.sender.username = a.authorUsername;
        aResp.timestamp       = a.timestamp;
        aResp.role            = a.isFaq ? "FAQ" : "";  // flag FAQ answers
        sender->send(aResp);
    }
}

// ── helpers ───────────────────────────────────────────────────────────────────
std::string ForumService::generateId()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    std::ostringstream oss;
    oss << std::hex << dis(gen);
    return oss.str();
}

std::string ForumService::currentTimestamp()
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&t), "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

void ForumService::sendError(const std::string& reason, std::shared_ptr<Session> sender)
{
    Message m; m.type = MessageType::ERROR; m.text = reason;
    sender->send(m);
}

void ForumService::sendOk(const std::string& text, std::shared_ptr<Session> sender)
{
    Message m; m.type = MessageType::QA_FAQ; m.text = text;
    sender->send(m);
}