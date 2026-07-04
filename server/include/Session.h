#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <queue>
#include <string>
#include "Message.h"

using boost::asio::ip::tcp;

// Forward declarations — avoids circular include with Server.h
class Server;
class Dispatcher;

// ─────────────────────────────────────────────────────────────────────────────
//  Session
//  Owns one TCP socket (one connected client).
//  Responsibilities:
//    - Read raw bytes from the socket, delimit by '\n'
//    - Deserialize bytes → Message, hand to Dispatcher
//    - Accept Message objects via send(), serialize and write to socket
//    - Manage a write queue so concurrent sends never corrupt the wire
//    - Report its authenticated userId to Server after login
// ─────────────────────────────────────────────────────────────────────────────
class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(tcp::socket socket, Server& server, Dispatcher& dispatcher);

    void start();

    // Called by Server / services to push a message to this client
    void send(const Message& msg);

    // Called by AuthService after successful login so Server can route by userId
    void setUserId(const std::string& id) { userId_ = id; }
    const std::string& userId() const     { return userId_; }

private:
    // ── read pipeline ──────────────────────────────────────────────────────
    void do_read();

    // ── write pipeline (queued so writes never overlap) ────────────────────
    void do_enqueue(std::shared_ptr<std::string> payload);
    void do_write_next();

    // ── error handling ─────────────────────────────────────────────────────
    void handle_error(const boost::system::error_code& ec);

    // ── members ────────────────────────────────────────────────────────────
    tcp::socket                              socket_;
    boost::asio::streambuf                   buffer_;
    Server&                                  server_;
    Dispatcher&                              dispatcher_;
    std::string                              userId_;      // empty until authenticated

    std::queue<std::shared_ptr<std::string>> writeQueue_;
    bool                                     writing_ = false;
};