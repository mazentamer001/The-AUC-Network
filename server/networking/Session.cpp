#include "Session.h"
#include <iostream>

Session::Session(tcp::socket socket): socket_(std::move(socket)) {} //The server passes the accepted socket into the constructor server's socket moves to Session::socket_

void Session::start() {  //start the session
    std::cout << "Client connected: " << socket_.remote_endpoint().address().to_string() << ":" << socket_.remote_endpoint().port() << std::endl;
    do_read();           //The session waits for messages from the client
}

void Session::do_read() {
    auto self = shared_from_this();     //<-- hardets line in the code, it is a pointer to the current object, and owns the object. As long as self exists, the object cannot be destroyed, this happens as async_read_until is an async function, it does not wait and returns immedietly, also if no one owns the object C++ can destroy it freely. So, if the client sends data, it will send it to an object that no longer exists, so we create this pointer that owns the object.
    boost::asio::async_read_until(      //read data until it encounters a newline
        socket_, buffer_, '\n',
        [this, self](boost::system::error_code ec, std::size_t) {
            if (!ec) {                                          //if no errors, process the message.
                std::istream is(&buffer_);                      //this treats the buffer like the standard input stream
                std::string line;               
                std::getline(is, line);                         //extract the input from buffer to line
                std::cout << "[recv] " << line << std::endl;    //print it out
                do_read();                                      // keep reading
            } else {
                handle_error(ec);
            }
        });
}

void Session::handle_error(const boost::system::error_code& ec) {       //we check 2 cases of errors
    if (ec == boost::asio::error::eof ||                                //eof means the client closed the Connections normally
        ec == boost::asio::error::connection_reset) {                   //connection_reset means the connection was terminated unexpectedly
        std::cout << "Client disconnected." << std::endl;
    } else {                                                            //else print whatever error happened
        std::cerr << "Session error: " << ec.message() << std::endl;
    }
    //socket closes when Session is destroyed (shared_ptr goes to 0)
}
