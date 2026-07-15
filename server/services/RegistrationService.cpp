#include "RegistrationService.h"
#include "InMemoryStore.h"
#include "UserRecord.h"
#include "Session.h"
#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include <regex>
#include <sstream>

RegistrationService::RegistrationService(InMemoryStore& store) : store_(store) {}

void RegistrationService::handleRegister(const Message& msg, std::shared_ptr<Session> sender)
{
    //validate all fields
    std::string error = validate(msg);
    if (!error.empty()) { sendError(error, sender); return; }

    //check uniqueness against store
    if (store_.findUserByUsername(msg.username))
    { sendError("Username already taken", sender); return; }

    if (store_.findUserByEmail(msg.email))
    { sendError("Email already registered", sender); return; }

    if (store_.findUserByUniversityId(msg.universityId))
    { sendError("University ID already registered", sender); return; }

    //build the record
    UserRecord user;
    user.userId = generateUserId();
    user.username = msg.username;
    user.displayName = msg.displayName;
    user.email = msg.email;
    user.passwordHash = hashPassword(msg.password);
    user.universityId = msg.universityId;
    user.role = Role::USER;          //never trust client-supplied role
    user.bio = msg.bio;             //optional, may be empty
    user.profilePicUrl = "";                 //set later via ProfileService
    user.major = msg.major;         //optional, may be empty
    user.year = yearFromString(msg.year);   //optional, defaults to UNSPECIFIED
    user.interests = msg.interests; //optional, may be empty

    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream ts;
    ts << std::put_time(std::gmtime(&t), "%Y-%m-%dT%H:%M:%SZ");
    user.createdAt = ts.str();

    if (!store_.addUser(user))
    { sendError("Registration failed — please try again", sender); return; }

    std::cout << "New user registered: " << user.username << std::endl;
    sendSuccess(user.username, sender);
}

std::string RegistrationService::validate(const Message& msg)
{
    if (msg.username.empty())
        return "Username is required";

    if (msg.displayName.empty())
        return "Display name is required";

    //basic email format check
    if (msg.email.find('@') == std::string::npos ||
        msg.email.find('.') == std::string::npos)
        return "Invalid email address";

    if (msg.password.size() < 8)
        return "Password must be at least 8 characters";

    //university ID: exactly 9 digits, starts with 900
    const std::regex uniIdPattern(R"(^900\d{6}$)");
    if (!std::regex_match(msg.universityId, uniIdPattern))
        return "University ID must be 9 digits and start with 900";

    return ""; //good
}

//password hashing
//NOTE: this is a placeholder SHA-style hash
//Replace with bcrypt (e.g. libbcrypt) when we implement database
std::string RegistrationService::hashPassword(const std::string& plain)
{
    std::hash<std::string> hasher;
    std::ostringstream oss;
    oss << std::hex << hasher(plain);
    return oss.str();
}

//UUID generation
std::string RegistrationService::generateUserId()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);

    const char* hex = "0123456789abcdef";
    std::string uuid = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";
    for (char& c : uuid)
    {
        if (c == 'x')      c = hex[dis(gen)];
        else if (c == 'y') c = hex[(dis(gen) & 0x3) | 0x8];
    }
    return uuid;
}

void RegistrationService::sendSuccess(const std::string& username, std::shared_ptr<Session> sender)
{
    Message resp;
    resp.type = MessageType::AUTH_RESPONSE;
    resp.text = "Registration successful. Welcome, " + username + "!";
    sender->send(resp);
}

void RegistrationService::sendError(const std::string& reason, std::shared_ptr<Session> sender)
{
    Message resp;
    resp.type = MessageType::ERROR;
    resp.text = reason;
    sender->send(resp);
}