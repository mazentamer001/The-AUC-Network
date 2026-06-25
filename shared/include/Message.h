#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>

class Message
{
    std::string sender;
    std::string content;

 public:
    Message(std::string, std::string);
};

#endif