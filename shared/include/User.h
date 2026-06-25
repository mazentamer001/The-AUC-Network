#ifndef USER_H
#define USER_H

#include <string>

class User
{
    std::string username;
    std::string password;

 public:
    User(std::string, std::string);
};

#endif