#include <QApplication>
#include <QWidget>
#include <QDebug>
#include <boost/asio.hpp>
#include <thread>

#include "Client.h"
#include "Message.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    window.resize(500, 300);
    window.show();

    boost::asio::io_context io;

    Client client(io, "127.0.0.1", "12345");

    client.setOnConnected([&]()
    {
        // build a test Message instead of a raw string
        Message msg;
        msg.type            = MessageType::CHAT_PUBLIC;
        msg.sender.userId   = "u001";
        msg.sender.username = "Alice";
        msg.sender.role     = "student";
        msg.roomId          = "room_general";
        msg.text            = "Hello from client!";
        msg.mediaUrl        = "";
        msg.timestamp       = "";   // server will stamp this later

        client.send(msg);
    });

    client.setOnMessage([](const Message& msg)
    {
        // runs on network thread — safe for qDebug, not for Qt widgets yet
        qDebug() << "[recv]"
                 << msg.sender.username
                 << ":" << msg.text;
    });

    std::thread networkThread([&]()
    {
        io.run();
    });

    int result = app.exec();

    io.stop();
    networkThread.join();

    return result;
}