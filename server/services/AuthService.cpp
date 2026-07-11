#include "AuthService.h"
#include "database/Database.h"
#include "models/UserRecord.h"
#include "AuthToken.h"
#include "Server.h"
#include "Session.h"
#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>


//creates AuthService and gives it access to the in database
AuthService::AuthService(Database& store) : store_(store){}

//logs a user into the system
void AuthService::handleLogin(const Message& msg, std::shared_ptr<Session> sender)
{
    //username and password cant be empty
    if (msg.username.empty() || msg.password.empty())
    { sendError("Username/email and password are required", sender); return; }

    //accept login by username OR email
    std::optional<UserRecord> userOpt = store_.findUserByUsername(msg.username);
    if (!userOpt)
        userOpt = store_.findUserByEmail(msg.username); //try as email

    //if not found send an error
    if (!userOpt)
    { sendError("Invalid credentials", sender); return; }

    //verify password (compares hashed input with hashed pass in database)
    if (userOpt->passwordHash != hashPassword(msg.password))
    { sendError("Invalid credentials", sender); return; } //same message as above for security w kda

    //Build token which expires in 24 hours
    auto now = std::chrono::system_clock::now().time_since_epoch();
    long long nowSec = std::chrono::duration_cast<std::chrono::seconds>(now).count();

    AuthToken token;
    token.sessionId = generateSessionId();      //sessionID
    token.userId = userOpt->userId;             //userID
    token.username = userOpt->username;         //username
    token.displayName = userOpt->displayName;   //display name
    token.role = roleToString(userOpt->role);   //role
    token.expiresAt = nowSec + (60 * 60 * 24);  // 24 hours

    store_.addSession(token);                   //store the token

    // kick any existing session for this account
    if (server_) {
        auto existing = server_->findSessionByUserId(userOpt->userId);
        if (existing && existing != sender) {
            existing->disconnect();
        }
    }

    sender->setUserId(userOpt->userId);                             //stores the logged-in user's ID inside the Session
    if (server_) server_->registerUser(userOpt->userId, sender);    //tell Server which userId maps to this socket

    std::cout << "User logged in: " << userOpt->username << std::endl;
    sendSuccess(token.sessionId, token.role, token.displayName, sender);  //tell the client login succeeded

    //broadcast presence
    Message presence;
    presence.type              = MessageType::PRESENCE;
    presence.sender.userId     = userOpt->userId;
    presence.sender.username   = userOpt->username;
    presence.displayName       = userOpt->displayName;
    presence.bio               = userOpt->bio;
    if (server_) server_->broadcast(presence, sender);

}

//logs the user out
void AuthService::handleLogout(const Message& msg, std::shared_ptr<Session> sender)
{
    store_.removeSession(msg.token);                            //remove token from database
    if (server_) server_->unregisterUser(sender->userId());     //Remove user from server user map
    sender->setUserId("");                                      //clear the session's stored ID

    //send a message
    Message resp;
    resp.type = MessageType::AUTH_RESPONSE;
    resp.text = "Logged out successfully";
    sender->send(resp);
    std::cout << "User logged out: " << msg.sender.username << std::endl;
}

//checks if a session token is valid
bool AuthService::validateToken(const std::string& sessionId)
{
    if (sessionId.empty()) return false;
    return store_.findSession(sessionId).has_value();
    // findSession already handles expiry — returns nullopt if expired
}

//generates a random session ID through a random number generator
std::string AuthService::generateSessionId()
{
    std::random_device              rd;
    std::mt19937_64                 gen(rd());
    std::uniform_int_distribution<uint8_t> dis(0, 255);

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (int i = 0; i < 16; ++i)          //16 bytes = 32 hex chars
        oss << std::setw(2) << (int)dis(gen);
    return oss.str();
}

//password hash (must match RegistrationService exactly)
std::string AuthService::hashPassword(const std::string& plain)
{
    std::hash<std::string> hasher;
    std::ostringstream oss;
    oss << std::hex << hasher(plain);
    return oss.str();
}

//sends a successful login message to the client
void AuthService::sendSuccess(const std::string& sessionId, const std::string& role, const std::string& displayName, std::shared_ptr<Session> sender)
{
    Message resp;
    resp.type        = MessageType::AUTH_RESPONSE;
    resp.token       = sessionId;   //client stores this and attaches to all future messages
    resp.text        = "Login successful";
    resp.displayName = displayName;
    resp.role        = role;
    sender->send(resp);
}

void AuthService::sendError(const std::string& reason, std::shared_ptr<Session> sender)
{
    Message resp;
    resp.type = MessageType::ERROR;
    resp.text = reason;
    sender->send(resp);
}

void AuthService::setServer(Server& server)
{
    server_ = &server;
}