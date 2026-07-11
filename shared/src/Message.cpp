#include "Message.h"
#include <nlohmann/json.hpp>
#include <stdexcept>

using json = nlohmann::json;

std::string Message::typeToString(MessageType t)
{
    switch (t) {
    case MessageType::AUTH_LOGIN:       return "AUTH_LOGIN";
    case MessageType::AUTH_REGISTER:    return "AUTH_REGISTER";
    case MessageType::AUTH_LOGOUT:      return "AUTH_LOGOUT";
    case MessageType::AUTH_RESPONSE:    return "AUTH_RESPONSE";
    case MessageType::PROFILE_GET:      return "PROFILE_GET";
    case MessageType::PROFILE_EDIT:     return "PROFILE_EDIT";
    case MessageType::CHAT_PUBLIC:      return "CHAT_PUBLIC";
    case MessageType::CHAT_PRIVATE:     return "CHAT_PRIVATE";
    case MessageType::CHAT_CREATE:      return "CHAT_CREATE";
    case MessageType::CHAT_HISTORY:     return "CHAT_HISTORY";
    case MessageType::CHAT_SUMMARIZE:   return "CHAT_SUMMARIZE";
    case MessageType::PRESENCE:         return "PRESENCE";
    case MessageType::JOIN:             return "JOIN";
    case MessageType::LEAVE:            return "LEAVE";
    case MessageType::MARKET_POST:      return "MARKET_POST";
    case MessageType::MARKET_DELETE:    return "MARKET_DELETE";
    case MessageType::MARKET_SEARCH:    return "MARKET_SEARCH";
    case MessageType::MARKET_INQUIRY:   return "MARKET_INQUIRY";
    case MessageType::MATERIAL_UPLOAD:  return "MATERIAL_UPLOAD";
    case MessageType::MATERIAL_REPORT:  return "MATERIAL_REPORT";
    case MessageType::MATERIAL_LIST:    return "MATERIAL_LIST";
    case MessageType::MATERIAL_GET:     return "MATERIAL_GET";
    case MessageType::QA_QUESTION:      return "QA_QUESTION";
    case MessageType::QA_ANSWER:        return "QA_ANSWER";
    case MessageType::QA_FAQ:           return "QA_FAQ";
    case MessageType::FORUM_UPVOTE:    return "FORUM_UPVOTE";
    case MessageType::FORUM_DOWNVOTE:  return "FORUM_DOWNVOTE";
    case MessageType::QA_GET_ALL:       return "QA_GET_ALL";
    case MessageType::QA_GET_ONE:       return "QA_GET_ONE";
    case MessageType::ERROR:            return "ERROR";
    default:                            return "UNKNOWN";
    }
}

MessageType Message::stringToType(const std::string& s)
{
    if (s == "AUTH_LOGIN")      return MessageType::AUTH_LOGIN;
    if (s == "AUTH_REGISTER")   return MessageType::AUTH_REGISTER;
    if (s == "AUTH_LOGOUT")     return MessageType::AUTH_LOGOUT;
    if (s == "AUTH_RESPONSE")   return MessageType::AUTH_RESPONSE;
    if (s == "PROFILE_GET")     return MessageType::PROFILE_GET;
    if (s == "PROFILE_EDIT")    return MessageType::PROFILE_EDIT;
    if (s == "CHAT_PUBLIC")     return MessageType::CHAT_PUBLIC;
    if (s == "CHAT_PRIVATE")    return MessageType::CHAT_PRIVATE;
    if (s == "CHAT_CREATE")     return MessageType::CHAT_CREATE;
    if (s == "CHAT_HISTORY")    return MessageType::CHAT_HISTORY;
    if (s == "CHAT_SUMMARIZE")  return MessageType::CHAT_SUMMARIZE;
    if (s == "PRESENCE")        return MessageType::PRESENCE;
    if (s == "JOIN")            return MessageType::JOIN;
    if (s == "LEAVE")           return MessageType::LEAVE;
    if (s == "MARKET_POST")     return MessageType::MARKET_POST;
    if (s == "MARKET_DELETE")   return MessageType::MARKET_DELETE;
    if (s == "MARKET_SEARCH")   return MessageType::MARKET_SEARCH;
    if (s == "MARKET_INQUIRY")  return MessageType::MARKET_INQUIRY;
    if (s == "MATERIAL_UPLOAD") return MessageType::MATERIAL_UPLOAD;
    if (s == "MATERIAL_REPORT") return MessageType::MATERIAL_REPORT;
    if (s == "MATERIAL_LIST")   return MessageType::MATERIAL_LIST;
    if (s == "MATERIAL_GET")    return MessageType::MATERIAL_GET;
    if (s == "QA_QUESTION")     return MessageType::QA_QUESTION;
    if (s == "QA_ANSWER")       return MessageType::QA_ANSWER;
    if (s == "QA_FAQ")          return MessageType::QA_FAQ;
    if (s == "FORUM_UPVOTE")   return MessageType::FORUM_UPVOTE;
    if (s == "FORUM_DOWNVOTE")  return MessageType::FORUM_DOWNVOTE;
    if (s == "QA_GET_ALL")      return MessageType::QA_GET_ALL;
    if (s == "QA_GET_ONE")      return MessageType::QA_GET_ONE;
    if (s == "ERROR")           return MessageType::ERROR;
    if (s == "FORUM_UPVOTE")    return MessageType::FORUM_UPVOTE;
    if (s == "FORUM_DOWNVOTE")  return MessageType::FORUM_DOWNVOTE;
    return MessageType::UNKNOWN;
}

//converts a Message into a json
std::string Message::serialize() const
{
    json j;
    j["type"]         = typeToString(type);
    j["token"]        = token;
    j["username"]     = username;
    j["password"]     = password;
    j["email"]        = email;
    j["displayName"]  = displayName;
    j["universityId"] = universityId;
    j["role"]         = role;
    j["roomId"]       = roomId;
    j["recipientId"]  = recipientId;
    j["text"]         = text;
    j["timestamp"]    = timestamp;
    j["title"]        = title;
    j["price"]        = price;
    j["mediaUrl"]     = mediaUrl;
    j["filename"]     = filename;
    j["parentId"]     = parentId;
    j["bio"]          = bio;
    j["profilePicUrl"]= profilePicUrl;
    j["sender"]["userId"]   = sender.userId;
    j["sender"]["username"] = sender.username;
    j["sender"]["role"]     = sender.role;
    return j.dump() + "\n";
}

//converts a json into a message object
Message Message::deserialize(const std::string& raw)
{
    json j;
    try { j = json::parse(raw); }
    catch (const json::parse_error& e) {
        throw std::invalid_argument(
            std::string("Message::deserialize: invalid JSON: ") + e.what());
    }
    if (!j.is_object())
        throw std::invalid_argument("Message::deserialize: expected JSON object");

    auto get = [&](const std::string& key) -> std::string {
        if (j.contains(key) && j[key].is_string())
            return j[key].get<std::string>();
        return "";
    };

    Message msg;
    msg.type          = stringToType(get("type"));
    msg.token         = get("token");
    msg.username      = get("username");
    msg.password      = get("password");
    msg.email         = get("email");
    msg.displayName   = get("displayName");
    msg.universityId  = get("universityId");
    msg.role          = get("role");
    msg.roomId        = get("roomId");
    msg.recipientId   = get("recipientId");
    msg.text          = get("text");
    msg.timestamp     = get("timestamp");
    msg.title         = get("title");
    msg.price         = get("price");
    msg.mediaUrl      = get("mediaUrl");
    msg.filename      = get("filename");
    msg.parentId      = get("parentId");
    msg.bio           = get("bio");
    msg.profilePicUrl = get("profilePicUrl");
    if (j.contains("sender") && j["sender"].is_object()) {
        msg.sender.userId   = j["sender"].value("userId",   "");
        msg.sender.username = j["sender"].value("username", "");
        msg.sender.role     = j["sender"].value("role",     "");
    }
    return msg;
}