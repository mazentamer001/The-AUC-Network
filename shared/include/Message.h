#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>

enum class MessageType {
    CHAT_PUBLIC,
    CHAT_PRIVATE,
    PRESENCE,
    MARKET_POST,
    MARKET_INQUIRY,
    MATERIAL_UPLOAD,
    MATERIAL_REPORT,
    QA_QUESTION,
    QA_ANSWER,
    QA_FAQ,
    JOIN,
    LEAVE,
    ERROR
};

struct SenderInfo {
    QString userId;
    QString username;
    QString role;        // "student" / "professor" / "admin"
};

struct Message {
    MessageType type;
    SenderInfo  sender;
    QString     roomId;
    QString     text;
    QString     mediaUrl;
    QString     timestamp;  // ISO 8601, stamped by server on receive

    std::string        serialize()   const;
    static Message     deserialize(const std::string& json);

private:
    static QString     typeToString(MessageType t);
    static MessageType stringToType(const QString& s);
};

#endif