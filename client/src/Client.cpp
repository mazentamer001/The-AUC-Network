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

void Client::setOnMessage(std::function<void(const Message&)> callback)
{
    onMessage_ = std::move(callback);
}

void Client::send(const Message& msg)
{
    auto payload = std::make_shared<std::string>(msg.serialize());
    do_send(payload);
}

void Client::do_resolve(const std::string& host, const std::string& port)
{
    resolver_.async_resolve(
        host, port,
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
        socket_, endpoints,
        [this](boost::system::error_code ec, const tcp::endpoint&)
        {
            if (!ec)
            {
                std::cout << "Connected!\n";
                do_read();              //start listening immediately
                if (onConnected_)
                    onConnected_();
            }
            else
                std::cerr << "Connect error: " << ec.message() << '\n';
        });
}

void Client::do_send(std::shared_ptr<std::string> payload)
{
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(*payload),
        [payload](boost::system::error_code ec, std::size_t)
        {
            if (!ec)
                std::cout << "[sent] " << *payload;
            else
                std::cerr << "Write error: " << ec.message() << '\n';
        });
}

void Client::do_read()
{
    boost::asio::async_read_until(
        socket_, buffer_, '\n',
        [this](boost::system::error_code ec, std::size_t)
        {
            if (!ec)
            {
                std::istream is(&buffer_);
                std::string line;
                std::getline(is, line);

                try
                {
                    Message msg = Message::deserialize(line);  //parse into Message

                    if (onMessage_)
                        onMessage_(msg);    //hand off to Qt layer (runs on network thread) and use Qt::QueuedConnection when connecting to UI slots)
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Bad message: " << e.what() << '\n';
                }

                do_read();  // keep listening
            }
            else if (ec == boost::asio::error::eof)
                std::cout << "Server disconnected.\n";
            else
                std::cerr << "Read error: " << ec.message() << '\n';
        });
}