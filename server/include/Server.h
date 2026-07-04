#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include "Message.h"

using boost::asio::ip::tcp;

class Session;
class Dispatcher;

// ─────────────────────────────────────────────────────────────────────────────
//  Server
//  Responsibilities:
//    - Own the tcp::acceptor, loop async_accept
//    - Spawn a Session per connection, hand it the Dispatcher
//    - Track live sessions (weak_ptr — sessions own themselves)
//    - Provide routing: broadcast / sendToRoom / sendTo(userId)
//    - Allow AuthService to register a userId → Session mapping after login
// ─────────────────────────────────────────────────────────────────────────────
class Server
{
public:
    Server(boost::asio::io_context& io,
           unsigned short            port,
           Dispatcher&               dispatcher);

    // ── routing API (called by services) ───────────────────────────────────
    // Send to every connected session except optional exclude
    void broadcast(const Message& msg,
                   std::shared_ptr<Session> exclude = nullptr);

    // Send to every session in a specific room except optional exclude
    void sendToRoom(const std::string&       roomId,
                    const Message&           msg,
                    std::shared_ptr<Session> exclude = nullptr);

    // Send to one specific authenticated user
    void sendTo(const std::string& userId, const Message& msg);

    // ── session registry (called by Session / AuthService) ─────────────────
    void addSession(std::shared_ptr<Session> session);
    void registerUser(const std::string& userId,
                      std::shared_ptr<Session> session);
    void unregisterUser(const std::string& userId);

private:
    void do_accept();

    // Removes expired weak_ptrs — call while holding sessionsMutex_
    void sweep();

    tcp::acceptor    acceptor_;
    Dispatcher&      dispatcher_;

    std::mutex       sessionsMutex_;

    // All live sessions (weak so Session lifetime is self-managed)
    std::vector<std::weak_ptr<Session>> sessions_;

    // userId → session for direct routing after authentication
    std::unordered_map<std::string, std::weak_ptr<Session>> userMap_;
};