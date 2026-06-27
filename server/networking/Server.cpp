#include "Server.h"
#include "Session.h"

Server::Server(boost::asio::io_context& io, unsigned short port)
    : acceptor_(io, tcp::endpoint(tcp::v4(), port))
{
    std::cout << "Server listening on port " << port << " ..." << std::endl;
    do_accept();
}

void Server::do_accept()
{
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
            if (!ec)
            {
                auto session = std::make_shared<Session>(std::move(socket), *this); // ← pass Server ref
                addSession(session);
                session->start();
            }
            else
            {
                std::cerr << "Accept error: " << ec.message() << std::endl;
            }
            do_accept();
        });
}

void Server::addSession(std::shared_ptr<Session> session)
{
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    sessions_.push_back(session);   // stored as weak_ptr — vector accepts shared_ptr implicitly
}

void Server::broadcast(const Message& msg, std::shared_ptr<Session> exclude)
{
    std::lock_guard<std::mutex> lock(sessionsMutex_);

    // sweep dead sessions out while we're iterating anyway
    sessions_.erase(
        std::remove_if(sessions_.begin(), sessions_.end(),
            [](const std::weak_ptr<Session>& w) { return w.expired(); }),
        sessions_.end());

    for (auto& weak : sessions_)
    {
        if (auto session = weak.lock())     // lock() gives shared_ptr or nullptr if dead
        {
            if (session != exclude)         // don't echo back to the sender
                session->send(msg);
        }
    }
}