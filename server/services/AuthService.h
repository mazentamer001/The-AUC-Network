#pragma once
#include <memory>
#include <string>
#include "Message.h"

class Session;
class Server;
class InMemoryStore;

// ─────────────────────────────────────────────────────────────────────────────
//  AuthService
//  Handles AUTH_LOGIN and AUTH_LOGOUT messages.
//  Also exposes validateToken() which every other service relies on via
//  Dispatcher::requireAuth().
//
//  Login accepts either username or email in msg.username field.
//  On success:
//    - Generates a sessionId (32-char hex)
//    - Stores AuthToken in InMemoryStore (expires in 24 hours)
//    - Registers userId → Session mapping in Server for direct routing
//    - Sends AUTH_RESPONSE with the sessionId back to the client
// ─────────────────────────────────────────────────────────────────────────────
class AuthService
{
public:
    AuthService(InMemoryStore& store);
    void setServer(Server& server);

    void handleLogin (const Message& msg, std::shared_ptr<Session> sender);
    void handleLogout(const Message& msg, std::shared_ptr<Session> sender);

    // Called by Dispatcher before routing any authenticated message
    // Returns true if sessionId in token is valid and not expired
    bool validateToken(const std::string& sessionId);

private:
    std::string generateSessionId();
    std::string hashPassword(const std::string& plain); // must match Registration's hash

    void sendSuccess(const std::string& sessionId,
                     const std::string& role,
                     const std::string& displayName,
                     std::shared_ptr<Session> sender);
    void sendError  (const std::string& reason,
                     std::shared_ptr<Session> sender);

    InMemoryStore& store_;
    Server*        server_ = nullptr;
};