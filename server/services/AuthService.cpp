#include "AuthService.h"
#include "InMemoryStore.h"
#include "UserRecord.h"
#include "AuthToken.h"
#include "Server.h"
#include "Session.h"
#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

AuthService::AuthService(InMemoryStore& store)
    : store_(store), server_(nullptr)
{}

void AuthService::setServer(Server& server)
{
    server_ = &server;
}

// ─────────────────────────────────────────────────────────────────────────────
void AuthService::handleLogin(const Message& msg,
                               std::shared_ptr<Session> sender)
{
    if (msg.username.empty() || msg.password.empty())
    { sendError("Username/email and password are required", sender); return; }

    // Accept login by username OR email
    std::optional<UserRecord> userOpt = store_.findUserByUsername(msg.username);
    if (!userOpt)
        userOpt = store_.findUserByEmail(msg.username); // try as email

    if (!userOpt)
    { sendError("Invalid credentials", sender); return; }

    // Verify password
    if (userOpt->passwordHash != hashPassword(msg.password))
    { sendError("Invalid credentials", sender); return; } // same message — no info leak

    // Build token — expires in 24 hours
    auto now = std::chrono::system_clock::now().time_since_epoch();
    long long nowSec = std::chrono::duration_cast<std::chrono::seconds>(now).count();

    AuthToken token;
    token.sessionId   = generateSessionId();
    token.userId      = userOpt->userId;
    token.username    = userOpt->username;
    token.displayName = userOpt->displayName;
    token.role        = roleToString(userOpt->role);
    token.expiresAt   = nowSec + (60 * 60 * 24); // 24 hours

    store_.addSession(token);

    // Tell Server which userId maps to this socket
    sender->setUserId(userOpt->userId);
    if (server_) server_->registerUser(userOpt->userId, sender);

    std::cout << "User logged in: " << userOpt->username << std::endl;
    sendSuccess(token.sessionId, token.role, token.displayName, sender);
}

// ─────────────────────────────────────────────────────────────────────────────
void AuthService::handleLogout(const Message& msg,
                                std::shared_ptr<Session> sender)
{
    store_.removeSession(msg.token);
    if (server_) server_->unregisterUser(sender->userId());
    sender->setUserId("");

    Message resp;
    resp.type = MessageType::AUTH_RESPONSE;
    resp.text = "Logged out successfully";
    sender->send(resp);
    std::cout << "User logged out: " << msg.sender.username << std::endl;
}

// ── token validation ──────────────────────────────────────────────────────────
bool AuthService::validateToken(const std::string& sessionId)
{
    if (sessionId.empty()) return false;
    return store_.findSession(sessionId).has_value();
    // findSession already handles expiry — returns nullopt if expired
}

// ── session ID generation ─────────────────────────────────────────────────────
std::string AuthService::generateSessionId()
{
    std::random_device              rd;
    std::mt19937_64                 gen(rd());
    std::uniform_int_distribution<uint8_t> dis(0, 255);

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (int i = 0; i < 16; ++i)          // 16 bytes = 32 hex chars
        oss << std::setw(2) << (int)dis(gen);
    return oss.str();
}

// ── password hash (must match RegistrationService exactly) ───────────────────
std::string AuthService::hashPassword(const std::string& plain)
{
    std::hash<std::string> hasher;
    std::ostringstream oss;
    oss << std::hex << hasher(plain);
    return oss.str();
}

// ── responses ─────────────────────────────────────────────────────────────────
void AuthService::sendSuccess(const std::string& sessionId,
                               const std::string& role,
                               const std::string& displayName,
                               std::shared_ptr<Session> sender)
{
    Message resp;
    resp.type        = MessageType::AUTH_RESPONSE;
    resp.token       = sessionId;   // client stores this and attaches to all future messages
    resp.text        = "Login successful";
    resp.displayName = displayName;
    resp.role        = role;
    sender->send(resp);
}

void AuthService::sendError(const std::string& reason,
                             std::shared_ptr<Session> sender)
{
    Message resp;
    resp.type = MessageType::ERROR;
    resp.text = reason;
    sender->send(resp);
}