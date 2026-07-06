#pragma once
#include <string>

enum class Role { USER, ADMIN };

inline std::string roleToString(Role r)
{
    return r == Role::ADMIN ? "ADMIN" : "USER";
}

inline Role roleFromString(const std::string& s)
{
    return s == "ADMIN" ? Role::ADMIN : Role::USER;
}

// ─────────────────────────────────────────────────────────────────────────────
//  UserRecord
//  Stored in InMemoryStore. Never leaves the server — the client only ever
//  sees what services explicitly put into a response Message.
// ─────────────────────────────────────────────────────────────────────────────
struct UserRecord
{
    std::string userId;          // UUID generated on registration
    std::string username;        // unique, used for login
    std::string displayName;     // shown in UI
    std::string email;           // unique, used for login
    std::string passwordHash;    // bcrypt hash — never store plain
    std::string universityId;    // 9 digits, starts with 900
    Role role;            // USER by default, ADMIN set manually
    std::string bio;             // optional
    std::string profilePicUrl;   // empty until user uploads one
    std::string createdAt;       // ISO-8601 timestamp
};