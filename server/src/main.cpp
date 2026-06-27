#include "Server.h"
#include <iostream>

int main()
{
    try
    {
        boost::asio::io_context io;
        Server server(io, 12345);
        io.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Server fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}