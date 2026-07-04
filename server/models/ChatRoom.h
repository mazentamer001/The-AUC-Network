#pragma once
#include <string>
#include <vector>

enum class RoomType { PUBLIC, GROUP, DIRECT };

struct ChatMessage {
    std::string messageId;
    std::string roomId;
    std::string senderUserId;
    std::string senderUsername;
    std::string text;
    std::string mediaUrl;
    std::string timestamp;
};

struct ChatRoom {
    std::string            roomId;
    std::string            name;
    RoomType               type;
    std::string            creatorId;
    std::vector<std::string> memberIds;   // empty = public (anyone can join)
    std::vector<ChatMessage> history;     // last 100 messages
    std::string            createdAt;
};