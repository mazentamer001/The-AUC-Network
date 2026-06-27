#include "Client.h"
#include <iostream>

Client::Client(boost::asio::io_context& io, const std::string& host, const std::string& port): socket_(io), resolver_(io)
{
    do_resolve(host, port);
}

void Client::setOnConnected(std::function<void()> callback)
{
    onConnected_ = std::move(callback);
}

void Client::send(const std::string& message)
{
    auto buffer = std::make_shared<std::string>(message);
    do_send(buffer);
}

void Client::do_resolve(const std::string& host, const std::string& port)
{
    resolver_.async_resolve(
        host,
        port,
        [this](boost::system::error_code ec, tcp::resolver::results_type endpoints)
        {
            if (!ec)
                do_connect(endpoints);
            else
                std::cerr << "Resolve error: " << ec.message() << '\n';
        });
}

void Client::do_connect(const tcp::resolver::results_type& endpoints)
{
    boost::asio::async_connect(
        socket_,
        endpoints,
        [this](boost::system::error_code ec, const tcp::endpoint&)
        {
            if (!ec)
            {
                std::cout << "Connected!\n";
                if (onConnected_)
                    onConnected_();
            }
            else
                std::cerr << "Connect error: " << ec.message() << '\n';
        });
}

void Client::do_send(std::shared_ptr<std::string> message)
{
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(*message),
        [message](boost::system::error_code ec,
                  std::size_t)
        {
            if (!ec)
                std::cout << "[sent] " << *message;
            else
                std::cerr << "Write error: " << ec.message() << '\n';
        });
}