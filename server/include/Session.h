#ifndef SESSION_H
#define SESSION_H

#include <memory>
#include <boost/asio.hpp>
using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {  //enable_shared_from_this allows the object to create a shared_ptr pointing to itself which is needed later on
 private:
    tcp::socket              socket_;     //this is the socket that comminucates with one client (each session has exactly one socket belonging to one client)
    boost::asio::streambuf   buffer_;     //this temporarly stores incoming bytes from client

    void do_read();
    void handle_error(const boost::system::error_code&);

 public:
    explicit Session(tcp::socket);       //explicit is used to prevent the compiler from doing implicit type conversions
    void start();
};

#endif
