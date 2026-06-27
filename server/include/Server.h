#ifndef SERVER_H
#define SERVER_H

#include <memory>
#include <iostream>
#include <boost/asio.hpp>
using boost::asio::ip::tcp;

class Server {
 private:
    tcp::acceptor acceptor_;  //this waits for incoming connections, it listens on a port, waits for clients, and gives each client its own socket
    void do_accept();
 public:
    Server(boost::asio::io_context&, unsigned short);
};

#endif