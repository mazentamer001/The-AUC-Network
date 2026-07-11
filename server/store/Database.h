#pragma once
#include <sqlite3.h>
#include <mutex>
#include <optional>
#include <string>
#include "models/UserRecord.h"

// ─────────────────────────────────────────────────────────────────────────────
//  Database
//  Thin wrapper around SQLite for persistent user storage.
//  All other data (chat, marketplace, forum, files) remains in-memory for now.
// ─────────────────────────────────────────────────────────────────────────────
class Database
{
public:
    explicit Database(const std::string& path);
    ~Database();

    bool addUser(const UserRecord& user);
    std::optional<UserRecord> findUserById(const std::string& userId);
    std::optional<UserRecord> findUserByUsername(const std::string& username);
    std::optional<UserRecord> findUserByEmail(const std::string& email);
    std::optional<UserRecord> findUserByUniversityId(const std::string& uniId);
    bool updateUser(const std::string& userId, const UserRecord& patch);

private:
    void initSchema();
    UserRecord rowToUser(sqlite3_stmt* stmt);
    std::optional<UserRecord> queryOne(const std::string& sql, const std::string& param);

    sqlite3*   db_;
    std::mutex mutex_;
};
