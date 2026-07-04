#include <QApplication>
#include <boost/asio.hpp>
#include <thread>

#include "Client.h"
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    boost::asio::io_context io;
    Client client(io, "127.0.0.1", "12345");

    MainWindow window(&client);
    window.show();

    std::thread networkThread([&]() {
        io.run();
    });

    int result = app.exec();

    io.stop();

    if (networkThread.joinable())
        networkThread.join();

    return result;
}
