#include "store/Database.h"
#include <stdexcept>
#include <iostream>

Database::Database(const std::string& path)
{
    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK)
        throw std::runtime_error("Failed to open database: " + std::string(sqlite3_errmsg(db_)));
    initSchema();
}

Database::~Database()
{
    if (db_) sqlite3_close(db_);
}

void Database::initSchema()
{
    const char* sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "  userId        TEXT PRIMARY KEY,"
        "  username      TEXT UNIQUE NOT NULL,"
        "  displayName   TEXT,"
        "  email         TEXT UNIQUE NOT NULL,"
        "  passwordHash  TEXT NOT NULL,"
        "  universityId  TEXT UNIQUE NOT NULL,"
        "  role          TEXT NOT NULL,"
        "  bio           TEXT,"
        "  profilePicUrl TEXT,"
        "  createdAt     TEXT"
        ");";

    char* errMsg = nullptr;
    if (sqlite3_exec(db_, sql, nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        std::string err = errMsg ? errMsg : "unknown error";
        sqlite3_free(errMsg);
        throw std::runtime_error("Failed to create users table: " + err);
    }
}

UserRecord Database::rowToUser(sqlite3_stmt* stmt)
{
    UserRecord u;
    auto col = [&](int i) -> std::string {
        const unsigned char* text = sqlite3_column_text(stmt, i);
        return text ? reinterpret_cast<const char*>(text) : "";
    };
    u.userId         = col(0);
    u.username       = col(1);
    u.displayName    = col(2);
    u.email          = col(3);
    u.passwordHash   = col(4);
    u.universityId   = col(5);
    u.role           = roleFromString(col(6));
    u.bio            = col(7);
    u.profilePicUrl  = col(8);
    u.createdAt      = col(9);
    return u;
}

bool Database::addUser(const UserRecord& user)
{
    std::lock_guard<std::mutex> lock(mutex_);

    const char* sql =
        "INSERT INTO users (userId, username, displayName, email, passwordHash, "
        "universityId, role, bio, profilePicUrl, createdAt) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text(stmt, 1, user.userId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user.username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, user.displayName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, user.email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, user.passwordHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, user.universityId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, roleToString(user.role).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, user.bio.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 9, user.profilePicUrl.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 10, user.createdAt.c_str(), -1, SQLITE_TRANSIENT);

    bool ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok; // returns false on UNIQUE constraint violation (dup username/email/uniId)
}

std::optional<UserRecord> Database::queryOne(const std::string& sql, const std::string& param)
{
    std::lock_guard<std::mutex> lock(mutex_);

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return std::nullopt;

    sqlite3_bind_text(stmt, 1, param.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<UserRecord> result;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        result = rowToUser(stmt);

    sqlite3_finalize(stmt);
    return result;
}

std::optional<UserRecord> Database::findUserById(const std::string& userId)
{
    return queryOne("SELECT * FROM users WHERE userId = ?;", userId);
}

std::optional<UserRecord> Database::findUserByUsername(const std::string& username)
{
    return queryOne("SELECT * FROM users WHERE username = ?;", username);
}

std::optional<UserRecord> Database::findUserByEmail(const std::string& email)
{
    return queryOne("SELECT * FROM users WHERE email = ?;", email);
}

std::optional<UserRecord> Database::findUserByUniversityId(const std::string& uniId)
{
    return queryOne("SELECT * FROM users WHERE universityId = ?;", uniId);
}

bool Database::updateUser(const std::string& userId, const UserRecord& patch)
{
    // sparse patch: only overwrite fields the caller actually set (non-empty)
    std::lock_guard<std::mutex> lock(mutex_);

    const char* sql =
        "UPDATE users SET "
        "displayName   = CASE WHEN ? != '' THEN ? ELSE displayName END, "
        "bio           = CASE WHEN ? != '' THEN ? ELSE bio END, "
        "profilePicUrl = CASE WHEN ? != '' THEN ? ELSE profilePicUrl END, "
        "passwordHash  = CASE WHEN ? != '' THEN ? ELSE passwordHash END, "
        "username      = CASE WHEN ? != '' THEN ? ELSE username END "
        "WHERE userId = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text(stmt, 1, patch.displayName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, patch.displayName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, patch.bio.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, patch.bio.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, patch.profilePicUrl.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, patch.profilePicUrl.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, patch.passwordHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, patch.passwordHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 9, patch.username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 10, patch.username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 11, userId.c_str(), -1, SQLITE_TRANSIENT);

    bool ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db_) > 0;
    sqlite3_finalize(stmt);
    return ok;
}
