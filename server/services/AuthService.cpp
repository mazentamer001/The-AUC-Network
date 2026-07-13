#include "AuthService.h"
#include "store/InMemoryStore.h"
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
AuthService::AuthService(InMemoryStore& store) : store_(store){}

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
    try {
        if (server_) {
            auto existing = server_->findSessionByUserId(userOpt->userId);
            if (existing && existing != sender) {
                existing->disconnect();
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Session kick error: " << e.what() << "\n";
    }

    sender->setUserId(userOpt->userId);                             //stores the logged-in user's ID inside the Session
    if (server_) server_->registerUser(userOpt->userId, sender);    //tell Server which userId maps to this socket

    std::cout << "User logged in: " << userOpt->username << std::endl;
    sendSuccess(token.sessionId, token.role, token.displayName, sender);  //tell the client login succeeded


    auto rooms = store_.getPublicRooms();
    for (auto& room : rooms) {
        Message roomMsg;
        roomMsg.type   = MessageType::JOIN;
        roomMsg.roomId = room.roomId;
        roomMsg.text   = room.name;
        roomMsg.role   = "PUBLIC";
        sender->send(roomMsg);
    }


    auto listings = store_.searchListings("");  // empty = all active
    for (auto& l : listings) {
        Message m;
        m.type            = MessageType::MARKET_POST;
        m.parentId        = l.listingId;
        m.title           = l.title;
        m.price           = l.price;
        m.text            = l.description;
        m.mediaUrl        = l.mediaUrl;
        m.sender.userId   = l.sellerUserId;
        m.sender.username = l.sellerUsername;
        m.timestamp       = l.createdAt;
        sender->send(m);
    }

    auto questions = store_.getAllQuestions();
    for (auto& q : questions) {
        Message m;
        m.type            = MessageType::QA_QUESTION;
        m.parentId        = q.questionId;
        m.title           = q.title;
        m.text            = q.text;
        m.sender.username = q.authorUsername;
        m.timestamp       = q.timestamp;
        sender->send(m);
    }

    auto files = store_.getAllFiles();
    for (auto& f : files) {
        Message m;
        m.type            = MessageType::MATERIAL_UPLOAD;
        m.parentId        = f.fileId;
        m.filename        = f.filename;
        m.mediaUrl        = f.url;
        m.text            = f.fileSize;
        m.sender.userId   = f.uploaderUserId;
        m.sender.username = f.uploaderUsername;
        m.timestamp       = f.uploadedAt;
        sender->send(m);
    }

    // 1. Send all registered users and online users to the new client (populates their sidebar)
    auto users = store_.getAllUsers();
    for (auto& u : users) {
        if (u.userId == sender->userId()) continue;
        Message presence;
        presence.type            = MessageType::PRESENCE;
        presence.sender.userId   = u.userId;
        presence.sender.username = u.username;
        presence.displayName     = u.displayName;
        presence.bio             = u.bio;
        sender->send(presence);
    }

    auto userRooms = store_.getRoomsForUser(userOpt->userId);
    for (auto& room : userRooms) {
        if (room.type == RoomType::PUBLIC)
            continue; // already sent via getPublicRooms() above

        std::string memberList;
        for (size_t i = 0; i < room.memberIds.size(); ++i) {
            memberList += room.memberIds[i];
            if (i + 1 < room.memberIds.size()) memberList += ",";
        }

        Message roomMsg;
        roomMsg.type     = MessageType::JOIN;
        roomMsg.roomId   = room.roomId;
        roomMsg.text     = room.name;
        roomMsg.role     = (room.type == RoomType::DIRECT) ? "DIRECT" : "GROUP";
        roomMsg.mediaUrl = memberList;
        sender->send(roomMsg);
    }

   if (server_) {
        auto onlineUsers = server_->getOnlineUserIds();
        for (auto& uid : onlineUsers) {
            if (uid == sender->userId()) continue;
            auto userOpt = store_.findUserById(uid);
            if (!userOpt) continue;
            bool away = server_->isUserAway(uid);
            Message statusMsg;
            statusMsg.type            = away ? MessageType::USER_AWAY : MessageType::USER_ONLINE;
            statusMsg.sender.userId   = userOpt->userId;
            statusMsg.sender.username = userOpt->username;
            statusMsg.displayName     = userOpt->displayName;
            sender->send(statusMsg);
        }
    }

    // 2. Broadcast USER_ONLINE to everyone else (updates their sidebar dot)
    Message online;
    online.type            = MessageType::USER_ONLINE;
    online.sender.userId   = userOpt->userId;
    online.sender.username = userOpt->username;
    online.displayName     = userOpt->displayName;
    if (server_) server_->broadcast(online, sender);

    // 3. Send new client their own USER_ONLINE so their dot shows green too
    online.sender.userId = userOpt->userId;
    sender->send(online);

}

//logs the user out
void AuthService::handleLogout(const Message& msg, std::shared_ptr<Session> sender)
{
    std::string userId = sender->userId();   //grab before it's cleared below

    store_.removeSession(msg.token);                            //remove token from database
    if (server_) server_->unregisterUser(userId);                //Remove user from server user map
    sender->setUserId("");                                       //clear the session's stored ID

    if (server_ && !userId.empty()) {
        auto userOpt = store_.findUserById(userId);
        Message offline;
        offline.type            = MessageType::USER_OFFLINE;
        offline.sender.userId   = userId;
        offline.sender.username = userOpt ? userOpt->username    : msg.sender.username;
        offline.displayName     = userOpt ? userOpt->displayName : "";
        server_->broadcast(offline, sender);
    }

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
   auto tokenOpt = store_.findSession(sessionId);

   Message resp;
    resp.type              = MessageType::AUTH_RESPONSE;
    resp.token             = sessionId;
    resp.text              = "Login successful";
    resp.displayName       = displayName;
    resp.role              = role;
    resp.sender.userId     = tokenOpt ? tokenOpt->userId   : sender->userId();
    resp.sender.username   = tokenOpt ? tokenOpt->username : "";
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