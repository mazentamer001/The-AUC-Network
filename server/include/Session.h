#ifndef SESSION_H
#define SESSION_H

#include <memory>
#include <boost/asio.hpp>
#include "Message.h"
using boost::asio::ip::tcp;

class Server;

class Session : public std::enable_shared_from_this<Session> {  //enable_shared_from_this allows the object to create a shared_ptr pointing to itself which is needed later on
 private:
    tcp::socket              socket_;     //this is the socket that comminucates with one client (each session has exactly one socket belonging to one client)
    boost::asio::streambuf   buffer_;     //this temporarly stores incoming bytes from client
    Server&                server_; 

    void do_read();
    void do_write(std::shared_ptr<std::string> payload);
    void handle_message(const Message& msg); 
    void handle_error(const boost::system::error_code&);

 public:
    explicit Session(tcp::socket, Server& server);       //explicit is used to prevent the compiler from doing implicit type conversions
    void start();
    void send(const Message& msg); 
};

#endif
