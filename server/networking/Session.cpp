#include "Session.h"
#include "Server.h"
#include "Dispatcher.h"
#include <iostream>

//The Session is what lives in the same computer as the server, if there are 50 Clients there should be roughly 50 Sessions
//The Client does not contact the server directly, it gets through a session first which is this file, then goes to the server

//constructor
Session::Session(tcp::socket socket, Server& server, Dispatcher& dispatcher)
    : socket_(std::move(socket))
    , server_(server)
    , dispatcher_(dispatcher)
{}

void Session::start()
{
    std::cout << "Client connected: "
              << socket_.remote_endpoint().address().to_string()
              << ":" << socket_.remote_endpoint().port()
              << std::endl;
    do_read();
}

//public send
void Session::send(const Message& msg)
{
    //serialize on whatever thread calls send(), then hand to queue
    do_enqueue(std::make_shared<std::string>(msg.serialize()));
}

//reads from client 
void Session::do_read()
{
    auto self = shared_from_this(); //shared ptr keeps object alive
    boost::asio::async_read_until(  //read from sokect, store incoming bytes in buffer, stop when u encounter a newline
        socket_, buffer_, '\n',
        [this, self](boost::system::error_code ec, std::size_t) //this function runs later "callback", sizet is the number of bytes read
        {
            if (ec) { handle_error(ec); return; }

            std::istream is(&buffer_);      //read from input buffer
            std::string  line;              
            std::getline(is, line);         //extract one complete line

            try
            {
                Message msg = Message::deserialize(line);       //convert JSON into message
                dispatcher_.dispatch(msg, shared_from_this());  //give it to dispatcher
            }
            catch (const std::exception& e)
            {
                std::cerr << "Bad message from "
                          << socket_.remote_endpoint().address().to_string()
                          << " : " << e.what() << std::endl;
                //malformed JSON, keep the connection alive, just skip this message
            }

            do_read(); // keep listening
        });
}

//just sends messages to the queue
void Session::do_enqueue(std::shared_ptr<std::string> payload)      //payload is the message we want to send
{
    //Must run on the io_context thread to avoid data races on writeQueue_.
    //post() guarantees that even if send() is called from another thread,
    //all queue manipulation happens serially inside the io_context, so we avoid data races
    auto self = shared_from_this();
    boost::asio::post(socket_.get_executor(),
        [this, self, payload]()
        {
            writeQueue_.push(payload);  //add to queue
            if (!writing_)              //if it is not wrriting then write the next
                do_write_next();
        });
}

//what actually sends the messages one by one, sends bytes through the socket to the client.
void Session::do_write_next()
{
    if (writeQueue_.empty()) { writing_ = false; return; }      //if there is nothing in the queue then sit idle

    writing_ = true;                         //mark that a write is in progress
    auto payload = writeQueue_.front();      //take the first in the queue
    writeQueue_.pop();                       //remove the first in the queue

    auto self = shared_from_this();
    boost::asio::async_write(       //send every byte to the payload
        socket_,
        boost::asio::buffer(*payload),
        [this, self, payload](boost::system::error_code ec, std::size_t)
        {
            if (ec) { handle_error(ec); return; }
            do_write_next(); // chain: write the next queued payload
        });
}

//error handling
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