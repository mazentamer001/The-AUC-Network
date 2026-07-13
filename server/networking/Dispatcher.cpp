#include "Dispatcher.h"
#include "Session.h"
#include "Server.h"
#include "Message.h"
#include "services/AuthService.h"
#include "services/RegistrationService.h"
#include "services/ProfileService.h"
#include "services/ChatService.h"
#include "services/MarketplaceService.h"
#include "services/ForumService.h"
#include "services/FileStorageService.h"
#include <iostream>

Dispatcher::Dispatcher(AuthService&         auth,
                       RegistrationService& registration,
                       ProfileService&      profile,
                       ChatService&         chat,
                       MarketplaceService&  marketplace,
                       ForumService&        forum,
                       FileStorageService&  fileStorage)
    : auth_(auth), registration_(registration), profile_(profile)
    , chat_(chat), marketplace_(marketplace)
    , forum_(forum), fileStorage_(fileStorage)
{}

void Dispatcher::dispatch(const Message& msg, std::shared_ptr<Session> sender)
{
    //for debugging
    std::cout << "Dispatcher received type: " 
              << Message::typeToString(msg.type) 
              << " (raw string from client: " << msg.role << ")" 
              << std::endl;

    //no token required
    switch (msg.type)
    {
    case MessageType::AUTH_LOGIN:
        auth_.handleLogin(msg, sender);    return;
    case MessageType::AUTH_REGISTER:
        registration_.handleRegister(msg, sender); return;
    default: break;
    }

    // ── everything else requires a valid token ─────────────────────────────
    if (!requireAuth(msg, sender)) return;

    switch (msg.type)
    {
    // ── auth ──────────────────────────────────────────────────────────────
    case MessageType::AUTH_LOGOUT:
        auth_.handleLogout(msg, sender);           break;

    // ── profile ───────────────────────────────────────────────────────────
    case MessageType::PROFILE_GET:
        profile_.handleGet(msg, sender);           break;
    case MessageType::PROFILE_EDIT:
        profile_.handleEdit(msg, sender);          break;

    // ── chat ──────────────────────────────────────────────────────────────
    case MessageType::CHAT_CREATE:
        chat_.handleCreate(msg, sender);           break;
    case MessageType::CHAT_PUBLIC:
        chat_.handlePublic(msg, sender);           break;
    case MessageType::CHAT_PRIVATE:
        chat_.handlePrivate(msg, sender);          break;
    case MessageType::CHAT_HISTORY:
        chat_.handleHistory(msg, sender);          break;
    case MessageType::JOIN:
        chat_.handleJoin(msg, sender);             break;
    case MessageType::LEAVE:
        chat_.handleLeave(msg, sender);            break;

    // ── marketplace ───────────────────────────────────────────────────────
    case MessageType::MARKET_POST:
        marketplace_.handlePost(msg, sender);      break;
    case MessageType::MARKET_DELETE:
        marketplace_.handleDelete(msg, sender);    break;
    case MessageType::MARKET_SEARCH:
        marketplace_.handleSearch(msg, sender);    break;
    case MessageType::MARKET_INQUIRY:
        marketplace_.handleInquiry(msg, sender);   break;

    // ── forum ─────────────────────────────────────────────────────────────
    case MessageType::QA_QUESTION:
        forum_.handleQuestion(msg, sender);        break;
    case MessageType::QA_ANSWER:
        forum_.handleAnswer(msg, sender);          break;
    case MessageType::QA_FAQ:
        forum_.handleFaq(msg, sender);             break;
    case MessageType::QA_GET_ALL:
        forum_.handleGetAll(msg, sender);          break;
    case MessageType::FORUM_UPVOTE:
        forum_.handleVote(msg, sender);          break;
    case MessageType::FORUM_DOWNVOTE:
        forum_.handleVote(msg, sender);          break;
    case MessageType::QA_GET_ONE:
        forum_.handleGetOne(msg, sender);          break;

    // ── files ─────────────────────────────────────────────────────────────
    case MessageType::MATERIAL_UPLOAD:
        fileStorage_.handleUpload(msg, sender);    break;
    case MessageType::MATERIAL_REPORT:
        fileStorage_.handleReport(msg, sender);    break;
    case MessageType::MATERIAL_LIST:
        fileStorage_.handleList(msg, sender);      break;
    case MessageType::MATERIAL_GET:
        fileStorage_.handleGetFile(msg, sender);   break;

    case MessageType::USER_AWAY:
        if (server_) server_->setUserAway(sender->userId(), true);
        if (server_) server_->broadcast(msg, sender);
        break;

    case MessageType::USER_ONLINE:
        if (server_) server_->setUserAway(sender->userId(), false);
        if (server_) server_->broadcast(msg, sender);
        break;


    default:
        std::cerr << "Dispatcher: unhandled type\n";
        sendError("Unknown message type", sender); break;
    }
}

bool Dispatcher::requireAuth(const Message& msg, std::shared_ptr<Session> sender)
{
    if (auth_.validateToken(msg.token)) return true;
    sendError("Unauthorized: missing or invalid token", sender);
    return false;
}

void Dispatcher::sendError(const std::string& reason, std::shared_ptr<Session> sender)
{
    Message err;
    err.type = MessageType::ERROR;
    err.text = reason;
    sender->send(err);
}