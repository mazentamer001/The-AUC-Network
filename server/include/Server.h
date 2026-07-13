#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include "Message.h"
#include <unordered_set>

using boost::asio::ip::tcp;

class Session;
class Dispatcher;

// ─────────────────────────────────────────────────────────────────────────────
//  Server
//  Responsibilities:
//    - Listen for incoming connections.
//    - Spawn a Session per connection, hand it the Dispatcher
//    - Track live sessions (weak_ptr — sessions own themselves)
//    - Provide routing: broadcast / sendToRoom / sendTo(userId)
//    - Allow AuthService to register a userId → Session mapping after login
// ─────────────────────────────────────────────────────────────────────────────
class Server
{
public:
    Server(boost::asio::io_context& io, unsigned short port, Dispatcher& dispatcher);
    void broadcast(const Message& msg, std::shared_ptr<Session> exclude = nullptr);

    void sendToRoom(const std::string& roomId, const Message& msg, std::shared_ptr<Session> exclude = nullptr);

    void sendTo(const std::string& userId, const Message& msg);

    void addSession(std::shared_ptr<Session> session);
    void registerUser(const std::string& userId, std::shared_ptr<Session> session);
    void unregisterUser(const std::string& userId);
    std::shared_ptr<Session> findSessionByUserId(const std::string& userId);
    std::vector<std::string> getOnlineUserIds();
    void setUserAway(const std::string& userId, bool away);
    bool isUserAway(const std::string& userId);

private:
    void do_accept();
    void sweep();

    tcp::acceptor    acceptor_;
    Dispatcher&      dispatcher_;

    std::mutex       sessionsMutex_;
    std::vector<std::weak_ptr<Session>> sessions_;
    std::unordered_map<std::string, std::weak_ptr<Session>> userMap_;
    std::unordered_set<std::string> awayUsers_;
};