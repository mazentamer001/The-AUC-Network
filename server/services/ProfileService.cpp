#include "services/ProfileService.h"
#include "store/InMemoryStore.h"
#include "Session.h"
#include <iostream>

ProfileService::ProfileService(InMemoryStore& store) : store_(store) {}

// ── get any user's profile ────────────────────────────────────────────────────
void ProfileService::handleGet(const Message& msg, std::shared_ptr<Session> sender)
{
    // If no userId supplied, return the caller's own profile
    std::string targetId = msg.recipientId.empty() ? sender->userId() : msg.recipientId;

    auto userOpt = store_.findUserById(targetId);
    if (!userOpt)
    { sendError("User not found", sender); return; }

    // Build response — never send passwordHash or universityId to client
    Message resp;
    resp.type          = MessageType::PROFILE_GET;
    resp.sender.userId = userOpt->userId;
    resp.username      = userOpt->username;
    resp.displayName   = userOpt->displayName;
    resp.role          = roleToString(userOpt->role);
    resp.bio           = userOpt->bio;
    resp.profilePicUrl = userOpt->profilePicUrl;
    sender->send(resp);
}

// ── edit own profile ──────────────────────────────────────────────────────────
void ProfileService::handleEdit(const Message& msg, std::shared_ptr<Session> sender)
{
    // Only edit your own profile — admins have no special edit rights here
    UserRecord patch;
    patch.displayName   = msg.displayName;
    patch.bio           = msg.bio;
    patch.profilePicUrl = msg.profilePicUrl;

    if (!store_.updateUser(sender->userId(), patch))
    { sendError("Failed to update profile", sender); return; }

    std::cout << "Profile updated: " << sender->userId() << "\n";

    Message resp;
    resp.type = MessageType::PROFILE_EDIT;
    resp.text = "Profile updated successfully";
    sender->send(resp);
}

void ProfileService::sendError(const std::string& reason, std::shared_ptr<Session> sender)
{
    Message m; m.type = MessageType::ERROR; m.text = reason;
    sender->send(m);
}