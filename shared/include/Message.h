#pragma once
#include <string>


//Message class is the common format the client and server use to communicate

enum class MessageType {
    //auth
AUTH_LOGIN, AUTH_REGISTER, AUTH_LOGOUT, AUTH_RESPONSE,
    //profile
PROFILE_GET, PROFILE_EDIT,
    //chat
CHAT_PUBLIC, CHAT_PRIVATE, CHAT_CREATE, CHAT_HISTORY,
    //presence
PRESENCE, JOIN, LEAVE,
    //marketplace
MARKET_POST, MARKET_DELETE, MARKET_SEARCH, MARKET_INQUIRY,
    //opportunities
OPP_POST, OPP_DELETE, OPP_SEARCH, OPP_INQUIRY,
    //files
MATERIAL_UPLOAD, MATERIAL_REPORT, MATERIAL_LIST, MATERIAL_GET,
    //forum
QA_QUESTION, QA_ANSWER, QA_FAQ, QA_GET_ALL, QA_GET_ONE,
    //system
ERROR, UNKNOWN,
    //forum votes
FORUM_UPVOTE, FORUM_DOWNVOTE,
    // User status
USER_ONLINE, USER_AWAY, USER_OFFLINE,

AI_SUMMARIZE_REQUEST, AI_SUMMARIZE_RESPONSE
};

struct SenderInfo {
std::string userId;
std::string username;
std::string role;
};

struct Message {
MessageType type    = MessageType::UNKNOWN;
SenderInfo  sender;

std::string token;
std::string username;
std::string password;
std::string email;
std::string displayName;
std::string universityId;
std::string role;

std::string roomId;
std::string recipientId;
std::string text;
std::string timestamp;

std::string title;
std::string price;
std::string mediaUrl;
std::string filename;
std::string parentId;
std::string bio;
std::string profilePicUrl;
std::string category;   // opportunity type: Job/Internship/Volunteer/Research/Other
std::string location;   // opportunity location, e.g. "Remote"

std::string major;      // profile: academic major, free text
std::string year;       // profile: academic year (Freshman/Sophomore/Junior/Senior/Grad)
std::string interests;  // profile: comma-separated free text

std::string        serialize()                     const;
static Message     deserialize(const std::string& raw);
static std::string typeToString(MessageType t);
static MessageType stringToType(const std::string& s);
};