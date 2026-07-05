#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>

enum class MessageType {
    // Auth
    AUTH_LOGIN, AUTH_REGISTER, AUTH_LOGOUT, AUTH_RESPONSE,
    // Profile
    PROFILE_GET, PROFILE_EDIT,
    // Chat
    CHAT_PUBLIC, CHAT_PRIVATE, CHAT_CREATE, CHAT_HISTORY,
    // Presence
    PRESENCE, JOIN, LEAVE,
    // Marketplace
    MARKET_POST, MARKET_DELETE, MARKET_SEARCH, MARKET_INQUIRY,
    // Files
    MATERIAL_UPLOAD, MATERIAL_REPORT, MATERIAL_LIST, MATERIAL_GET,
    // Forum
    QA_QUESTION, QA_ANSWER, QA_FAQ, QA_GET_ALL, QA_GET_ONE,
    // System
    ERROR, UNKNOWN,
    // Forum votes
    FORUM_UPVOTE, FORUM_DOWNVOTE
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

    std::string        serialize()                     const;
    static Message     deserialize(const std::string& raw);
    static std::string typeToString(MessageType t);
    static MessageType stringToType(const std::string& s);
};

#endif
// note: FORUM_UPVOTE and FORUM_DOWNVOTE added below UNKNOWN