#ifndef SERVER_H
#define SERVER_H

#include <memory>
#include <vector>
#include <mutex>
#include <iostream>
#include <boost/asio.hpp>
#include "Message.h"

using boost::asio::ip::tcp;

class Session;  // forward declare — Server and Session refer to each other

class Server {
private:
    tcp::acceptor                        acceptor_;
    std::vector<std::weak_ptr<Session>>  sessions_;   // ← weak so we don't keep dead sessions alive
    std::mutex                           sessionsMutex_;  // ← do_accept and broadcast run on the same
                                                          //   io thread but good habit for later

    void do_accept();

public:
    Server(boost::asio::io_context&, unsigned short);

    void broadcast(const Message& msg,
                   std::shared_ptr<Session> exclude = nullptr);  // ← exclude the sender
    void addSession(std::shared_ptr<Session> session);
};

#endif