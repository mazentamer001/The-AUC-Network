#pragma once

#include <boost/asio.hpp>
#include <functional>
#include <memory>
#include "Message.h"

using boost::asio::ip::tcp;

class Client
{
private:
    tcp::socket socket_;        //the clients one and only tcp connection
    tcp::resolver resolver_;    //the resolver converts ip addresses into addresses the socket can conenct to
    boost::asio::streambuf buffer_; 

    void do_read(); 
    void do_resolve(const std::string& host, const std::string& port);
    void do_connect(const tcp::resolver::results_type& endpoints);
    void do_send(std::shared_ptr<std::string> message);
    std::function<void()> onConnected_;
    std::function<void(const Message&)> onMessage_; 

public:
    Client(boost::asio::io_context& io, const std::string& host, const std::string& port);
    void send(const Message& msg);
    void setOnConnected(std::function<void()> callback);
    void setOnMessage(std::function<void(const Message&)> callback); 
};