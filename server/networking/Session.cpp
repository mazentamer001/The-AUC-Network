#include "Session.h"
#include "Server.h"
#include "Dispatcher.h"
#include <iostream>

// ─────────────────────────────────────────────────────────────────────────────
Session::Session(tcp::socket socket, Server& server, Dispatcher& dispatcher)
    : socket_(std::move(socket))
    , server_(server)
    , dispatcher_(dispatcher)
{}

// ─────────────────────────────────────────────────────────────────────────────
void Session::start()
{
    std::cout << "Client connected: "
              << socket_.remote_endpoint().address().to_string()
              << ":" << socket_.remote_endpoint().port()
              << std::endl;
    do_read();
}

// ── public send ───────────────────────────────────────────────────────────────
void Session::send(const Message& msg)
{
    // Serialize on whatever thread calls send(), then hand to queue
    do_enqueue(std::make_shared<std::string>(msg.serialize()));
}

// ── read pipeline ─────────────────────────────────────────────────────────────
void Session::do_read()
{
    auto self = shared_from_this();
    boost::asio::async_read_until(
        socket_, buffer_, '\n',
        [this, self](boost::system::error_code ec, std::size_t)
        {
            if (ec) { handle_error(ec); return; }

            std::istream is(&buffer_);
            std::string  line;
            std::getline(is, line);

            try
            {
                Message msg = Message::deserialize(line);
                dispatcher_.dispatch(msg, shared_from_this()); // ← one line, no switch
            }
            catch (const std::exception& e)
            {
                std::cerr << "Bad message from "
                          << socket_.remote_endpoint().address().to_string()
                          << " : " << e.what() << std::endl;
                // malformed JSON — keep the connection alive, just skip this message
            }

            do_read(); // keep listening
        });
}

// ── write pipeline ────────────────────────────────────────────────────────────
// Enqueue a payload. If nothing is currently being written, start the chain.
void Session::do_enqueue(std::shared_ptr<std::string> payload)
{
    // Must run on the io_context thread to avoid data races on writeQueue_.
    // post() guarantees that even if send() is called from another thread,
    // all queue manipulation happens serially inside the io_context.
    auto self = shared_from_this();
    boost::asio::post(socket_.get_executor(),
        [this, self, payload]()
        {
            writeQueue_.push(payload);
            if (!writing_)
                do_write_next();
        });
}

void Session::do_write_next()
{
    if (writeQueue_.empty()) { writing_ = false; return; }

    writing_      = true;
    auto payload  = writeQueue_.front();
    writeQueue_.pop();

    auto self = shared_from_this();
    boost::asio::async_write(
        socket_,
        boost::asio::buffer(*payload),
        [this, self, payload](boost::system::error_code ec, std::size_t)
        {
            if (ec) { handle_error(ec); return; }
            do_write_next(); // chain: write the next queued payload
        });
}

// ── error handling ────────────────────────────────────────────────────────────
void Session::handle_error(const boost::system::error_code& ec)
{
    if (ec == boost::asio::error::eof ||
        ec == boost::asio::error::connection_reset)
    {
        std::cout << "Client disconnected"
                  << (userId_.empty() ? "" : " (user: " + userId_ + ")")
                  << std::endl;
    }
    else
    {
        std::cerr << "Session error: " << ec.message() << std::endl;
    }

    // Clean up auth mapping so dead session isn't routed to
    if (!userId_.empty())
        server_.unregisterUser(userId_);
}