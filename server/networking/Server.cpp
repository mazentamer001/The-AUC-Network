#include "Server.h"
#include "Session.h"
#include "Dispatcher.h"
#include <algorithm>
#include <iostream>

//constructor we initialize the acceptor which accepts connection and the dispatcher which routes messages
Server::Server(boost::asio::io_context& io,
               unsigned short            port,
               Dispatcher&               dispatcher)
    : acceptor_(io, tcp::endpoint(tcp::v4(), port))
    , dispatcher_(dispatcher)
{
    std::cout << "Server listening on port " << port << " ..." << std::endl;
    do_accept();
}

//accept loop
void Server::do_accept()
{
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
            if (!ec)
            {
                auto session = std::make_shared<Session>(std::move(socket), *this, dispatcher_);
                addSession(session);
                session->start();
            }
            else
            {
                std::cerr << "Accept error: " << ec.message() << std::endl;
            }
            do_accept(); //always loop, always look for connections
        });
}

//when a new client connects, the server creates a Session object for that client
void Server::addSession(std::shared_ptr<Session> session)
{
    std::lock_guard<std::mutex> lock(sessionsMutex_);   //Mutex is a lock that prevents data races so no 2 threads can overwrite data in the same time
    sessions_.push_back(session);
}

//------ instead of just storing all sessions, it associates a user ID with a session ------
void Server::registerUser(const std::string& userId, std::shared_ptr<Session> session)
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
//--------------------------------------------------------------------------------------------

//sweep dead sessions
//removes every weak pointer that points to a destroyed session.
///a weak pointer is like the shared pointer but it doesnt own the object (doesnt know how to keep it alive)
void Server::sweep()
{
    sessions_.erase(
        std::remove_if(sessions_.begin(), sessions_.end(),
            [](const std::weak_ptr<Session>& w) { return w.expired(); }),  //expired returns true when the Session is dead
        sessions_.end());
}

//this takes the message to send and sends it to everyone but the sender
void Server::broadcast(const Message& msg, std::shared_ptr<Session> exclude)
{
    std::lock_guard<std::mutex> lock(sessionsMutex_);   //lock to prevent data races
    sweep();                                            //remove dead sessions as its meaningless to send to them
    for (auto& weak : sessions_)                        //loop over sessions
    {
        if (auto s = weak.lock())                       //creates a shared pointer from a weak pointer and since we just used sweep() this should normally return true as the object exists
            if (s != exclude)
                s->send(msg);
    }
}

//this does the exact same as broadcasting this should change to send to a specific room only
void Server::sendToRoom(const std::string& roomId, const Message& msg, std::shared_ptr<Session> exclude)
{
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    sweep();
    for (auto& weak : sessions_)
    {
        if (auto s = weak.lock())
            // Session exposes its roomId once we add room tracking (next step)
            //this should not send to everyone (like broadcast) it should send to everyone within a room
            if (s != exclude)
                s->send(msg);   // TODO: filter by s->roomId() == roomId
    }
}

//this sends to a specific user
void Server::sendTo(const std::string& userId, const Message& msg)
{
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    auto it = userMap_.find(userId);
    if (it == userMap_.end()) return;
    if (it == userMap_.end()) return; // user not connected — silently drop

    if (auto s = it->second.lock())
        s->send(msg);
    else
        userMap_.erase(it); // session died, clean up
}

std::shared_ptr<Session> Server::findSessionByUserId(const std::string& userId)
{
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    auto it = userMap_.find(userId);
    if (it == userMap_.end()) return nullptr;
    return it->second.lock();
}

std::vector<std::string> Server::getOnlineUserIds()
{
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    std::vector<std::string> result;
    for (auto& [userId, weak] : userMap_)
        if (!weak.expired())
            result.push_back(userId);
    return result;
}

void Server::setUserAway(const std::string& userId, bool away)
{
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    if (away) awayUsers_.insert(userId);
    else      awayUsers_.erase(userId);
}

bool Server::isUserAway(const std::string& userId)
{
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    return awayUsers_.count(userId) > 0;
}

// Server.cpp
void Server::sendToRoom(const std::vector<std::string>& memberIds, const Message& msg, std::shared_ptr<Session> exclude)
{
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    for (const auto& userId : memberIds)
    {
        auto it = userMap_.find(userId);
        if (it == userMap_.end()) continue;
        if (auto s = it->second.lock())
            if (s != exclude)
                s->send(msg);
        else
            userMap_.erase(it);
    }
}