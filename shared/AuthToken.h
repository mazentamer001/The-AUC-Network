#pragma once
#include <string>

// ─────────────────────────────────────────────────────────────────────────────
//  AuthToken
//  Lives in shared/ because both the server (stores it) and the client
//  (receives and attaches it to every message) need this shape.
// ─────────────────────────────────────────────────────────────────────────────
struct AuthToken
{
    std::string sessionId;    // random 32-char hex string
    std::string userId;
    std::string username;
    std::string displayName;
    std::string role;         // "USER" or "ADMIN"
    long long   expiresAt;    // unix timestamp (seconds)
};