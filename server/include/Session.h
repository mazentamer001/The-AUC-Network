#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <queue>
#include <string>
#include "Message.h"

using boost::asio::ip::tcp;
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

    void send(const Message& msg);
    void setUserId(const std::string& id) { userId_ = id; }
    const std::string& userId() const     { return userId_; }

private:
    void do_read();
    void do_enqueue(std::shared_ptr<std::string> payload);
    void do_write_next();
    void handle_error(const boost::system::error_code& ec);

    tcp::socket socket_;
    boost::asio::streambuf buffer_;
    Server& server_;
    Dispatcher& dispatcher_;
    std::string userId_; 

    std::queue<std::shared_ptr<std::string>> writeQueue_;
    bool writing_ = false;
};