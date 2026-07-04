#include "Server.h"
#include "Session.h"
#include "Dispatcher.h"
#include <algorithm>
#include <iostream>

// ─────────────────────────────────────────────────────────────────────────────
Server::Server(boost::asio::io_context& io,
               unsigned short            port,
               Dispatcher&               dispatcher)
    : acceptor_(io, tcp::endpoint(tcp::v4(), port))
    , dispatcher_(dispatcher)
{
    std::cout << "Server listening on port " << port << " ..." << std::endl;
    do_accept();
}

// ── accept loop ───────────────────────────────────────────────────────────────
void Server::do_accept()
{
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
            if (!ec)
            {
                auto session = std::make_shared<Session>(
                    std::move(socket), *this, dispatcher_);
                addSession(session);
                session->start();
            }
            else
            {
                std::cerr << "Accept error: " << ec.message() << std::endl;
            }
            do_accept(); // always loop
        });
}

// ── session registry ──────────────────────────────────────────────────────────
void Server::addSession(std::shared_ptr<Session> session)
{
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    sessions_.push_back(session); // stored as weak_ptr implicitly
}

void Server::registerUser(const std::string& userId,
                           std::shared_ptr<Session> session)
{
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    userMap_[userId] = session;
    std::cout << "User registered: " << userId << std::endl;
}

void Server::unregisterUser(const std::string& userId)
{
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    userMap_.erase(userId);
    std::cout << "User unregistered: " << userId << std::endl;
}

// ── sweep dead sessions ───────────────────────────────────────────────────────
// Must be called while holding sessionsMutex_
void Server::sweep()
{
    sessions_.erase(
        std::remove_if(sessions_.begin(), sessions_.end(),
            [](const std::weak_ptr<Session>& w) { return w.expired(); }),
        sessions_.end());
}

// ── routing ───────────────────────────────────────────────────────────────────
void Server::broadcast(const Message& msg, std::shared_ptr<Session> exclude)
{
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    sweep();
    for (auto& weak : sessions_)
    {
        if (auto s = weak.lock())
            if (s != exclude)
                s->send(msg);
    }
}

void Server::sendToRoom(const std::string&       roomId,
                         const Message&           msg,
                         std::shared_ptr<Session> exclude)
{
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    sweep();
    for (auto& weak : sessions_)
    {
        if (auto s = weak.lock())
            // Session exposes its roomId once we add room tracking (next step)
            // For now fall back to broadcast within the room filter
            if (s != exclude)
                s->send(msg);   // TODO: filter by s->roomId() == roomId
    }
}

void Server::sendTo(const std::string& userId, const Message& msg)
{
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    auto it = userMap_.find(userId);
    if (it == userMap_.end()) return; // user not connected — silently drop

    if (auto s = it->second.lock())
        s->send(msg);
    else
        userMap_.erase(it); // session died, clean up
}