#include "Server.h"
#include "Session.h"

Server::Server(boost::asio::io_context& io, unsigned short port) : acceptor_(io, tcp::endpoint(tcp::v4(), port)) {  //IPv4 (type of addressing scheme used to identify the machines), server listens on ONE port and waits for clients to connect on said port
    std::cout << "Server listening on port " << port << " ..." << std::endl;
    do_accept();
}

//This is the loop that waits for clients to connect
void Server::do_accept() {
    acceptor_.async_accept(     //this is an async function as we dont want to block the cpu while we are waiting for a connection.
        [this](boost::system::error_code ec, tcp::socket socket) {           //ec is the error if its empty (!ec) then everything worked, the socket is the brand new socket representing the client, every client has their own socket
            if (!ec) {  //if no errors, constracted a session and start it
                std::make_shared<Session>(std::move(socket))->start();       //std::move(socket) makes the LOCAL socket in server empty as there can only be one socket for each client so we move that socket to the client's session
            } else {    //error
                std::cerr << "Accept error: " << ec.message() << std::endl;
            }
            do_accept();  //begin waiting for the next client
        });
}