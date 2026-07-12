#pragma once
#include <memory>
#include "Message.h"

class Session;
class InMemoryStore;

// ─────────────────────────────────────────────────────────────────────────────
//  RegistrationService
//  Handles AUTH_REGISTER messages.
//
//  Validation rules:
//    - username    : non-empty, unique
//    - displayName : non-empty
//    - email       : unique, must contain '@' and '.'
//    - password    : minimum 8 characters (stored as bcrypt hash)
//    - universityId: exactly 9 digits, starts with "900"
//    - bio         : optional
//    - role        : always set to USER — never trust client-supplied role
// ─────────────────────────────────────────────────────────────────────────────

class RegistrationService
{
public:
    explicit RegistrationService(InMemoryStore& store);

    void handleRegister(const Message& msg, std::shared_ptr<Session> sender);

private:
    //returns an empty string if valid, or the error reason if not
    std::string validate(const Message& msg);

    //minimal password hash — replace with bcrypt when adding real DB
    std::string hashPassword(const std::string& plain);

    //generates a UUID-style userId
    std::string generateUserId();

    void sendSuccess(const std::string& username, std::shared_ptr<Session> sender);
    void sendError(const std::string& reason, std::shared_ptr<Session> sender);

    InMemoryStore& store_;
};