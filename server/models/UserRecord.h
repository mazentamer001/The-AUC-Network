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

// Academic year — stored as a string on the wire/DB (same pattern as Role),
// with a fixed set of canonical values driven by the UI dropdown.
enum class Year { FRESHMAN, SOPHOMORE, JUNIOR, SENIOR, GRAD, UNSPECIFIED };

inline std::string yearToString(Year y)
{
    switch (y) {
        case Year::FRESHMAN:  return "Freshman";
        case Year::SOPHOMORE: return "Sophomore";
        case Year::JUNIOR:    return "Junior";
        case Year::SENIOR:    return "Senior";
        case Year::GRAD:      return "Grad";
        default:              return "";
    }
}

inline Year yearFromString(const std::string& s)
{
    if (s == "Freshman")  return Year::FRESHMAN;
    if (s == "Sophomore") return Year::SOPHOMORE;
    if (s == "Junior")    return Year::JUNIOR;
    if (s == "Senior")    return Year::SENIOR;
    if (s == "Grad")      return Year::GRAD;
    return Year::UNSPECIFIED;
}

// ─────────────────────────────────────────────────────────────────────────────
//  UserRecord
//  Stored in Databse. Never leaves the server — the client only ever
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
    Role role;                   // USER by default, ADMIN set manually
    std::string bio;             // optional
    std::string profilePicUrl;   // empty until user uploads one
    std::string major;           // optional, free text
    Year        year = Year::UNSPECIFIED;   // optional, fixed dropdown
    std::string interests;       // optional, free text (comma-separated)
    std::string createdAt;       // ISO-8601 timestamp
};