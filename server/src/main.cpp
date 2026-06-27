#include"Session.h"
#include"Server.h"
#include <iostream>


int main() {
    try {
        boost::asio::io_context io;  //create the io context
        Server server(io, 12345);    //create a server tht listens on port 12345 via tcp::acceptor

        io.run();                    //run the io context
        
    } catch (const std::exception& e) {     //if we catch an error
        std::cerr << "Server fatal error: " << e.what() << std::endl;   //we print it and terminate
        return 1;
    }
    return 0;
}