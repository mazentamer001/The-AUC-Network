#include <gtest/gtest.h>
#include <stdexcept>
#include "Message.h"

TEST(MessageTest, PublicMessageSerializeAndDeserialize)
{
    Message original;

    original.type = MessageType::CHAT_PUBLIC;
    original.roomId = "general";
    original.text = "Hello everyone!";
    original.timestamp = "2026-07-14";

    original.sender.userId = "user1";
    original.sender.username = "judy";
    original.sender.role = "student";

    auto serialized = original.serialize();
    Message result = Message::deserialize(serialized);

    EXPECT_EQ(result.type, MessageType::CHAT_PUBLIC);
    EXPECT_EQ(result.roomId, "general");
    EXPECT_EQ(result.text, "Hello everyone!");
    EXPECT_EQ(result.timestamp, "2026-07-14");

    EXPECT_EQ(result.sender.userId, "user1");
    EXPECT_EQ(result.sender.username, "judy");
    EXPECT_EQ(result.sender.role, "student");
}

TEST(MessageTest, PrivateMessageSerializeAndDeserialize)
{
    Message original;

    original.type = MessageType::CHAT_PRIVATE;
    original.roomId = "private-room";
    original.text = "Private hello";
    original.timestamp = "2026-07-14";

    original.sender.userId = "user2";
    original.sender.username = "mazentamer";
    original.sender.role = "student";

    auto serialized = original.serialize();
    Message result = Message::deserialize(serialized);

    EXPECT_EQ(result.type, MessageType::CHAT_PRIVATE);
    EXPECT_EQ(result.roomId, "private-room");
    EXPECT_EQ(result.text, "Private hello");
    EXPECT_EQ(result.timestamp, "2026-07-14");

    EXPECT_EQ(result.sender.userId, "user2");
    EXPECT_EQ(result.sender.username, "mazentamer");
    EXPECT_EQ(result.sender.role, "student");
}

TEST(MessageTest, MissingFieldsBecomeEmpty)
{
    Message result = Message::deserialize(
        R"({"type":"CHAT_PUBLIC","text":"Hello"})"
    );

    EXPECT_EQ(result.type, MessageType::CHAT_PUBLIC);
    EXPECT_EQ(result.text, "Hello");
EXPECT_TRUE(result.roomId.isEmpty());
EXPECT_TRUE(result.timestamp.isEmpty());
EXPECT_TRUE(result.sender.userId.isEmpty());
EXPECT_TRUE(result.sender.username.isEmpty());
EXPECT_TRUE(result.sender.role.isEmpty());
}

TEST(MessageTest, InvalidJsonThrowsException)
{
    EXPECT_THROW(
        Message::deserialize("{invalid json}"),
        std::invalid_argument
    );
}

TEST(MessageTest, JsonArrayThrowsException)
{
    EXPECT_THROW(
        Message::deserialize(R"(["Hello", "World"])"),
        std::invalid_argument
    );
}
