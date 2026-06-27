#include <QApplication>
#include <QWidget>

#include <boost/asio.hpp>
#include <thread>

#include "Client.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.resize(500,300);
    window.show();

    boost::asio::io_context io;             //creates io context

    Client client(io, "127.0.0.1","12345"); //creates client (giving them an ip address and server port)

    client.setOnConnected([&]()             //when successfully connected execute the function send()
    {
        client.send("Hello from client\n");
    });

    std::thread networkingThread([&]()      //since Qt has its own loop and the io context has its own loop, Qt runs in the main thread and the io context runs on another thread
    {
        io.run();
    });

    int result = app.exec();

    io.stop();
    networkingThread.join();

    return result;
}