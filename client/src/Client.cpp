#include "Client.h"
#include <iostream>


//constructor, initializes the socket which is used to communicate with the server and the resolver which turns domain to ip
Client::Client(boost::asio::io_context& io, const std::string& host, const std::string& port): socket_(io), resolver_(io)
{
    do_resolve(host, port); //the connection process begins immedietly
}

//callbacks are functions that you give to another object so it can call it later (ex. when button is clicked, call this function)
void Client::setOnConnected(std::function<void()> callback)     //std::function<void()> stores any function that takes no parameters and returns nothing
{
    onConnected_ = std::move(callback);
}

void Client::setOnMessage(std::function<void(const Message&)> callback)
{
    onMessage_ = std::move(callback);
}

void Client::send(const Message& msg)
{
    auto payload = std::make_shared<std::string>(msg.serialize());  //we serialize the msg first into a JSON string as bytes are sent over networks not cpp objects and the serialized string is stored in a shared_ptr so it remains alive until the asynchronous write operation finishes, since async_write() returns immediately while the data is still being sent in the background
    do_send(payload);                                               //then we send the serialized messsage to the server
}

void Client::do_resolve(const std::string& host, const std::string& port)       
{
    resolver_.async_resolve(    //built in asio method, the resolver asks the OS to translate the hostname [NO CONNECTION IS MADE YET]
        host, port,
        [this](boost::system::error_code ec, tcp::resolver::results_type endpoints)     //endpoints is the result of the translation, for websites there may be several possible IP addresses, so we store them all in endpoints
        {
            if (!ec)
                do_connect(endpoints);  //if there is no error then we establish the connection, this is where the connection starts
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