#include "Session.h"
#include "Server.h"
#include <iostream>

Session::Session(tcp::socket socket, Server& server)
    : socket_(std::move(socket)), server_(server) {}

void Session::start()
{
    std::cout << "Client connected: "
              << socket_.remote_endpoint().address().to_string()
              << ":" << socket_.remote_endpoint().port() << std::endl;
    do_read();
}

// ── public send API ────────────────────────────────────────────────────────

void Session::send(const Message& msg)
{
    auto payload = std::make_shared<std::string>(msg.serialize());
    do_write(payload);
}

// ── write ──────────────────────────────────────────────────────────────────

void Session::do_write(std::shared_ptr<std::string> payload)
{
    auto self = shared_from_this();
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(*payload),
        [this, self, payload](boost::system::error_code ec, std::size_t)
        {
            if (ec)
                handle_error(ec);
        });
}

// ── read ───────────────────────────────────────────────────────────────────

void Session::do_read()
{
    auto self = shared_from_this();
    boost::asio::async_read_until(
        socket_, buffer_, '\n',
        [this, self](boost::system::error_code ec, std::size_t)
        {
            if (!ec)
            {
                std::istream is(&buffer_);
                std::string line;
                std::getline(is, line);

                try
                {
                    Message msg = Message::deserialize(line);
                    handle_message(msg);
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Bad message: " << e.what() << std::endl;
                }

                do_read();
            }
            else
            {
                handle_error(ec);
            }
        });
}

// ── message handler ────────────────────────────────────────────────────────

void Session::handle_message(const Message& msg)
{
    std::cout << "[" << msg.sender.username.toStdString() << "] "
              << msg.text.toStdString() << std::endl;

    switch (msg.type)
    {
        case MessageType::CHAT_PUBLIC:
            // broadcast to everyone in the same room except the sender
            server_.broadcast(msg, shared_from_this());
            break;

        case MessageType::JOIN:
            // broadcast join notification to everyone
            server_.broadcast(msg, shared_from_this());
            break;

        case MessageType::LEAVE:
            server_.broadcast(msg, shared_from_this());
            break;

        // private chat, marketplace, Q&A etc. — routing comes in later steps
        default:
            std::cout << "Unhandled message type from "
                      << msg.sender.username.toStdString() << std::endl;
            break;
    }
}

// ── error handler ──────────────────────────────────────────────────────────

void Session::handle_error(const boost::system::error_code& ec)
{
    if (ec == boost::asio::error::eof ||
        ec == boost::asio::error::connection_reset)
        std::cout << "Client disconnected." << std::endl;
    else
        std::cerr << "Session error: " << ec.message() << std::endl;
}