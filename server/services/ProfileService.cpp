#include "services/ProfileService.h"
#include "store/InMemoryStore.h"
#include "models/UserRecord.h"
#include "Session.h"
#include "Server.h"
#include <functional>
#include <iostream>
#include <sstream>

//constructor
ProfileService::ProfileService(InMemoryStore& store) : store_(store) {}

//get own profile
void ProfileService::handleGet(const Message& msg, std::shared_ptr<Session> sender)
{
    //recipientId empty = get own profile
    std::string targetId = msg.recipientId.empty() ? sender->userId() : msg.recipientId;

    auto userOpt = store_.findUserById(targetId);
    if (!userOpt) { sendError("User not found", sender); return; }

    Message resp;
    resp.type          = MessageType::PROFILE_GET;
    resp.sender.userId = userOpt->userId;
    resp.username      = userOpt->username;
    resp.displayName   = userOpt->displayName;
    resp.role          = roleToString(userOpt->role);
    resp.bio           = userOpt->bio;
    resp.profilePicUrl = userOpt->profilePicUrl;
    resp.email         = userOpt->email;
    resp.universityId  = userOpt->universityId;
    resp.major         = userOpt->major;
    resp.year          = yearToString(userOpt->year);
    resp.interests     = userOpt->interests;
    sender->send(resp);
}

//edit own profile
void ProfileService::handleEdit(const Message& msg, std::shared_ptr<Session> sender)
{
    auto userOpt = store_.findUserById(sender->userId());
    if (!userOpt) { sendError("User not found", sender); return; }

    UserRecord patch;

    //display name
    if (!msg.displayName.empty())
        patch.displayName = msg.displayName;

    //bio
    if (!msg.bio.empty())
        patch.bio = msg.bio;

    //profile pic
    if (!msg.profilePicUrl.empty())
        patch.profilePicUrl = msg.profilePicUrl;

    //major
    if (!msg.major.empty())
        patch.major = msg.major;

    //year
    if (!msg.year.empty())
        patch.year = yearFromString(msg.year);

    //interests
    if (!msg.interests.empty())
        patch.interests = msg.interests;

    //username change
    if (!msg.username.empty() && msg.username != userOpt->username) {
        //check uniqueness
        if (store_.findUserByUsername(msg.username)) {
            sendError("Username already taken", sender); return;
        }
        patch.username = msg.username;
    }

    //password change - msg.text = current password, msg.password = new password
    if (!msg.password.empty()) {
        if (msg.text.empty()) { sendError("Current password required", sender); return; }
        if (hashPassword(msg.text) != userOpt->passwordHash) {
            sendError("Current password is incorrect", sender); return;
        }
        if (msg.password.size() < 8) {
            sendError("Password must be at least 8 characters", sender); return;
        }
        patch.passwordHash = hashPassword(msg.password);
    }

   if (!store_.updateUser(sender->userId(), patch)) {
        sendError("Failed to update profile", sender); return;
    }

    std::cout << "Profile updated: " << sender->userId() << "\n";

    // re-fetch the fresh record and send it back in full, so the client
    // can repopulate its UI without a second round trip
    auto updated = store_.findUserById(sender->userId());
    if (!updated) { sendError("Failed to reload profile after update", sender); return; }

    // if the photo changed, let every other connected client know so their
    // sidebar avatar updates live too — reuses USER_ONLINE, which the client
    // already handles by updating the photo (and, as a side effect, marking
    // this user online; acceptable since editing your profile requires an
    // active connection anyway)
    if (!patch.profilePicUrl.empty() && server_) {
        Message photoUpdate;
        photoUpdate.type            = MessageType::USER_ONLINE;
        photoUpdate.sender.userId   = updated->userId;
        photoUpdate.sender.username = updated->username;
        photoUpdate.displayName     = updated->displayName;
        photoUpdate.profilePicUrl   = updated->profilePicUrl;
        server_->broadcast(photoUpdate, sender);
    }

    Message resp;
    resp.type          = MessageType::PROFILE_EDIT;
    resp.text          = "Profile updated successfully";
    resp.sender.userId = updated->userId;
    resp.username      = updated->username;
    resp.displayName   = updated->displayName;
    resp.role          = roleToString(updated->role);
    resp.bio           = updated->bio;
    resp.profilePicUrl = updated->profilePicUrl;
    resp.email         = updated->email;
    resp.universityId  = updated->universityId;
    resp.major         = updated->major;
    resp.year          = yearToString(updated->year);
    resp.interests     = updated->interests;
    sender->send(resp);
}

std::string ProfileService::hashPassword(const std::string& plain)
{
    std::hash<std::string> hasher;
    std::ostringstream oss;
    oss << std::hex << hasher(plain);
    return oss.str();
}

void ProfileService::sendError(const std::string& reason, std::shared_ptr<Session> sender)
{
    Message m; m.type = MessageType::ERROR; m.text = reason;
    sender->send(m);
}

//view someone else's profile — public-safe subset only, no email/universityId/password
void ProfileService::handleView(const Message& msg, std::shared_ptr<Session> sender)
{
    if (msg.recipientId.empty()) { sendError("recipientId is required", sender); return; }

    auto userOpt = store_.findUserById(msg.recipientId);
    if (!userOpt) { sendError("User not found", sender); return; }

    Message resp;
    resp.type           = MessageType::PROFILE_VIEW;
    resp.sender.userId  = userOpt->userId;
    resp.displayName    = userOpt->displayName;
    resp.username       = userOpt->username;
    resp.bio            = userOpt->bio;
    resp.profilePicUrl  = userOpt->profilePicUrl;
    resp.major          = userOpt->major;
    resp.year           = yearToString(userOpt->year);
    resp.interests      = userOpt->interests;
    // deliberately omitted: email, universityId, password, role
    sender->send(resp);
}