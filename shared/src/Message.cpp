#include "Message.h"
#include <stdexcept>

// ── helpers ────────────────────────────────────────────────────────────────

QString Message::typeToString(MessageType t)
{
    switch (t) {
        case MessageType::CHAT_PUBLIC:      return "CHAT_PUBLIC";
        case MessageType::CHAT_PRIVATE:     return "CHAT_PRIVATE";
        case MessageType::PRESENCE:         return "PRESENCE";
        case MessageType::MARKET_POST:      return "MARKET_POST";
        case MessageType::MARKET_INQUIRY:   return "MARKET_INQUIRY";
        case MessageType::MATERIAL_UPLOAD:  return "MATERIAL_UPLOAD";
        case MessageType::MATERIAL_REPORT:  return "MATERIAL_REPORT";
        case MessageType::QA_QUESTION:      return "QA_QUESTION";
        case MessageType::QA_ANSWER:        return "QA_ANSWER";
        case MessageType::QA_FAQ:           return "QA_FAQ";
        case MessageType::JOIN:             return "JOIN";
        case MessageType::LEAVE:            return "LEAVE";
        case MessageType::ERROR:            return "ERROR";
        default:                            return "UNKNOWN";
    }
}

MessageType Message::stringToType(const QString& s)
{
    if (s == "CHAT_PUBLIC")     return MessageType::CHAT_PUBLIC;
    if (s == "CHAT_PRIVATE")    return MessageType::CHAT_PRIVATE;
    if (s == "PRESENCE")        return MessageType::PRESENCE;
    if (s == "MARKET_POST")     return MessageType::MARKET_POST;
    if (s == "MARKET_INQUIRY")  return MessageType::MARKET_INQUIRY;
    if (s == "MATERIAL_UPLOAD") return MessageType::MATERIAL_UPLOAD;
    if (s == "MATERIAL_REPORT") return MessageType::MATERIAL_REPORT;
    if (s == "QA_QUESTION")     return MessageType::QA_QUESTION;
    if (s == "QA_ANSWER")       return MessageType::QA_ANSWER;
    if (s == "QA_FAQ")          return MessageType::QA_FAQ;
    if (s == "JOIN")            return MessageType::JOIN;
    if (s == "LEAVE")           return MessageType::LEAVE;
    if (s == "ERROR")           return MessageType::ERROR;
    throw std::invalid_argument("Unknown message type: " + s.toStdString());
}

// ── serialize ──────────────────────────────────────────────────────────────

std::string Message::serialize() const
{
    QJsonObject senderObj;
    senderObj["userId"]   = sender.userId;
    senderObj["username"] = sender.username;
    senderObj["role"]     = sender.role;

    QJsonObject root;
    root["type"]      = typeToString(type);
    root["sender"]    = senderObj;
    root["roomId"]    = roomId;
    root["text"]      = text;
    root["mediaUrl"]  = mediaUrl;
    root["timestamp"] = timestamp;

    // compact JSON + \n delimiter for async_read_until on the wire
    return QJsonDocument(root).toJson(QJsonDocument::Compact).toStdString() + "\n";
}

// ── deserialize ────────────────────────────────────────────────────────────

Message Message::deserialize(const std::string& json)
{
    QJsonDocument doc = QJsonDocument::fromJson(
        QByteArray::fromStdString(json));

    if (doc.isNull() || !doc.isObject())
        throw std::invalid_argument("Message::deserialize: invalid JSON");

    QJsonObject root      = doc.object();
    QJsonObject senderObj = root["sender"].toObject();

    Message msg;
    msg.type      = stringToType(root["type"].toString());
    msg.roomId    = root["roomId"].toString();
    msg.text      = root["text"].toString();
    msg.mediaUrl  = root["mediaUrl"].toString();
    msg.timestamp = root["timestamp"].toString();

    msg.sender.userId   = senderObj["userId"].toString();
    msg.sender.username = senderObj["username"].toString();
    msg.sender.role     = senderObj["role"].toString();

    return msg;
}