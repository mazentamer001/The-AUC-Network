#include "database/Database.h"
#include <filesystem>
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <algorithm>

namespace fs = std::filesystem;

static std::string nowIso() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&t), "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

// ─────────────────────────────────────────────────────────────────────────────
Database::Database(const std::string& path)
{
    fs::create_directories(fs::path(path).parent_path());
    db_ = std::make_unique<SQLite::Database>(path,
          SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    db_->exec("PRAGMA journal_mode=WAL;");
    db_->exec("PRAGMA foreign_keys=ON;");
    createTables();
    std::cout << "Database opened: " << path << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
void Database::createTables()
{
    db_->exec(R"(
        CREATE TABLE IF NOT EXISTS users (
            userId       TEXT PRIMARY KEY,
            username     TEXT UNIQUE NOT NULL,
            displayName  TEXT NOT NULL,
            email        TEXT UNIQUE NOT NULL,
            passwordHash TEXT NOT NULL,
            universityId TEXT UNIQUE NOT NULL,
            role         TEXT NOT NULL DEFAULT 'USER',
            bio          TEXT DEFAULT '',
            profilePicUrl TEXT DEFAULT '',
            createdAt    TEXT NOT NULL
        );

        CREATE TABLE IF NOT EXISTS sessions (
            sessionId   TEXT PRIMARY KEY,
            userId      TEXT NOT NULL,
            username    TEXT NOT NULL,
            displayName TEXT NOT NULL,
            role        TEXT NOT NULL,
            expiresAt   INTEGER NOT NULL
        );

        CREATE TABLE IF NOT EXISTS chat_rooms (
            roomId    TEXT PRIMARY KEY,
            name      TEXT NOT NULL,
            type      TEXT NOT NULL,
            creatorId TEXT NOT NULL,
            createdAt TEXT NOT NULL
        );

        CREATE TABLE IF NOT EXISTS chat_members (
            roomId TEXT NOT NULL,
            userId TEXT NOT NULL,
            PRIMARY KEY (roomId, userId)
        );

        CREATE TABLE IF NOT EXISTS chat_messages (
            messageId  TEXT PRIMARY KEY,
            roomId     TEXT NOT NULL,
            senderUserId TEXT NOT NULL,
            senderUsername TEXT NOT NULL,
            text       TEXT NOT NULL,
            mediaUrl   TEXT DEFAULT '',
            timestamp  TEXT NOT NULL
        );

        CREATE TABLE IF NOT EXISTS listings (
            listingId       TEXT PRIMARY KEY,
            sellerUserId    TEXT NOT NULL,
            sellerUsername  TEXT NOT NULL,
            title           TEXT NOT NULL,
            description     TEXT DEFAULT '',
            price           TEXT NOT NULL,
            mediaUrl        TEXT DEFAULT '',
            status          TEXT NOT NULL DEFAULT 'ACTIVE',
            createdAt       TEXT NOT NULL
        );

        CREATE TABLE IF NOT EXISTS forum_questions (
            questionId      TEXT PRIMARY KEY,
            authorUserId    TEXT NOT NULL,
            authorUsername  TEXT NOT NULL,
            title           TEXT NOT NULL,
            text            TEXT NOT NULL,
            upvotes         INTEGER DEFAULT 0,
            downvotes       INTEGER DEFAULT 0,
            timestamp       TEXT NOT NULL
        );

        CREATE TABLE IF NOT EXISTS forum_answers (
            answerId        TEXT PRIMARY KEY,
            questionId      TEXT NOT NULL,
            authorUserId    TEXT NOT NULL,
            authorUsername  TEXT NOT NULL,
            text            TEXT NOT NULL,
            isFaq           INTEGER DEFAULT 0,
            upvotes         INTEGER DEFAULT 0,
            downvotes       INTEGER DEFAULT 0,
            timestamp       TEXT NOT NULL
        );

        CREATE TABLE IF NOT EXISTS forum_votes (
            targetId  TEXT NOT NULL,
            userId    TEXT NOT NULL,
            voteType  TEXT NOT NULL,
            PRIMARY KEY (targetId, userId)
        );

        CREATE TABLE IF NOT EXISTS files (
            fileId          TEXT PRIMARY KEY,
            uploaderUserId  TEXT NOT NULL,
            uploaderUsername TEXT NOT NULL,
            filename        TEXT NOT NULL,
            url             TEXT NOT NULL,
            fileSize        TEXT DEFAULT '',
            flagged         INTEGER DEFAULT 0,
            flagReason      TEXT DEFAULT '',
            uploadedAt      TEXT NOT NULL
        );
    )");
}

// ─────────────────────────────────────────────────────────────────────────────
//  Row helpers
// ─────────────────────────────────────────────────────────────────────────────
UserRecord Database::rowToUser(SQLite::Statement& q)
{
    UserRecord u;
    u.userId        = q.getColumn("userId").getText();
    u.username      = q.getColumn("username").getText();
    u.displayName   = q.getColumn("displayName").getText();
    u.email         = q.getColumn("email").getText();
    u.passwordHash  = q.getColumn("passwordHash").getText();
    u.universityId  = q.getColumn("universityId").getText();
    u.role          = roleFromString(q.getColumn("role").getText());
    u.bio           = q.getColumn("bio").getText();
    u.profilePicUrl = q.getColumn("profilePicUrl").getText();
    u.createdAt     = q.getColumn("createdAt").getText();
    return u;
}

AuthToken Database::rowToToken(SQLite::Statement& q)
{
    AuthToken t;
    t.sessionId   = q.getColumn("sessionId").getText();
    t.userId      = q.getColumn("userId").getText();
    t.username    = q.getColumn("username").getText();
    t.displayName = q.getColumn("displayName").getText();
    t.role        = q.getColumn("role").getText();
    t.expiresAt   = q.getColumn("expiresAt").getInt64();
    return t;
}

ChatMessage Database::rowToMessage(SQLite::Statement& q)
{
    ChatMessage m;
    m.messageId      = q.getColumn("messageId").getText();
    m.roomId         = q.getColumn("roomId").getText();
    m.senderUserId   = q.getColumn("senderUserId").getText();
    m.senderUsername = q.getColumn("senderUsername").getText();
    m.text           = q.getColumn("text").getText();
    m.mediaUrl       = q.getColumn("mediaUrl").getText();
    m.timestamp      = q.getColumn("timestamp").getText();
    return m;
}

Listing Database::rowToListing(SQLite::Statement& q)
{
    Listing l;
    l.listingId      = q.getColumn("listingId").getText();
    l.sellerUserId   = q.getColumn("sellerUserId").getText();
    l.sellerUsername = q.getColumn("sellerUsername").getText();
    l.title          = q.getColumn("title").getText();
    l.description    = q.getColumn("description").getText();
    l.price          = q.getColumn("price").getText();
    l.mediaUrl       = q.getColumn("mediaUrl").getText();
    std::string st   = q.getColumn("status").getText();
    l.status = (st == "SOLD") ? ListingStatus::SOLD
             : (st == "DELETED") ? ListingStatus::DELETED
             : ListingStatus::ACTIVE;
    l.createdAt = q.getColumn("createdAt").getText();
    return l;
}

// ─────────────────────────────────────────────────────────────────────────────
//  USERS
// ─────────────────────────────────────────────────────────────────────────────
bool Database::addUser(const UserRecord& u)
{
    std::lock_guard<std::mutex> lock(mutex_);
    try {
        SQLite::Statement q(*db_,
            "INSERT INTO users (userId,username,displayName,email,passwordHash,"
            "universityId,role,bio,profilePicUrl,createdAt) "
            "VALUES (?,?,?,?,?,?,?,?,?,?)");
        q.bind(1,  u.userId);
        q.bind(2,  u.username);
        q.bind(3,  u.displayName);
        q.bind(4,  u.email);
        q.bind(5,  u.passwordHash);
        q.bind(6,  u.universityId);
        q.bind(7,  roleToString(u.role));
        q.bind(8,  u.bio);
        q.bind(9,  u.profilePicUrl);
        q.bind(10, u.createdAt);
        q.exec();
        return true;
    } catch (const SQLite::Exception& e) {
        std::cerr << "addUser: " << e.what() << "\n";
        return false;
    }
}

std::optional<UserRecord> Database::findUserById(const std::string& userId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    SQLite::Statement q(*db_, "SELECT * FROM users WHERE userId=?");
    q.bind(1, userId);
    if (q.executeStep()) return rowToUser(q);
    return std::nullopt;
}

std::optional<UserRecord> Database::findUserByUsername(const std::string& username)
{
    std::lock_guard<std::mutex> lock(mutex_);
    SQLite::Statement q(*db_, "SELECT * FROM users WHERE username=?");
    q.bind(1, username);
    if (q.executeStep()) return rowToUser(q);
    return std::nullopt;
}

std::optional<UserRecord> Database::findUserByEmail(const std::string& email)
{
    std::lock_guard<std::mutex> lock(mutex_);
    SQLite::Statement q(*db_, "SELECT * FROM users WHERE email=?");
    q.bind(1, email);
    if (q.executeStep()) return rowToUser(q);
    return std::nullopt;
}

std::optional<UserRecord> Database::findUserByUniversityId(const std::string& uniId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    SQLite::Statement q(*db_, "SELECT * FROM users WHERE universityId=?");
    q.bind(1, uniId);
    if (q.executeStep()) return rowToUser(q);
    return std::nullopt;
}

bool Database::updateUser(const std::string& userId, const UserRecord& patch)
{
    std::lock_guard<std::mutex> lock(mutex_);
    try {
        // build dynamic UPDATE only for non-empty fields
        std::string sql = "UPDATE users SET ";
        std::vector<std::string> parts;
        if (!patch.displayName.empty())   parts.push_back("displayName=?");
        if (!patch.bio.empty())           parts.push_back("bio=?");
        if (!patch.profilePicUrl.empty()) parts.push_back("profilePicUrl=?");
        if (!patch.passwordHash.empty())  parts.push_back("passwordHash=?");
        if (!patch.username.empty())      parts.push_back("username=?");
        if (parts.empty()) return true;

        for (size_t i = 0; i < parts.size(); i++)
            sql += parts[i] + (i+1 < parts.size() ? "," : "");
        sql += " WHERE userId=?";

        SQLite::Statement q(*db_, sql);
        int idx = 1;
        if (!patch.displayName.empty())   q.bind(idx++, patch.displayName);
        if (!patch.bio.empty())           q.bind(idx++, patch.bio);
        if (!patch.profilePicUrl.empty()) q.bind(idx++, patch.profilePicUrl);
        if (!patch.passwordHash.empty())  q.bind(idx++, patch.passwordHash);
        if (!patch.username.empty())      q.bind(idx++, patch.username);
        q.bind(idx, userId);
        q.exec();
        return true;
    } catch (const SQLite::Exception& e) {
        std::cerr << "updateUser: " << e.what() << "\n";
        return false;
    }
}

std::vector<UserRecord> Database::getAllUsers()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<UserRecord> result;
    SQLite::Statement q(*db_, "SELECT * FROM users");
    while (q.executeStep()) result.push_back(rowToUser(q));
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
//  SESSIONS
// ─────────────────────────────────────────────────────────────────────────────
void Database::addSession(const AuthToken& t)
{
    std::lock_guard<std::mutex> lock(mutex_);
    SQLite::Statement q(*db_,
        "INSERT OR REPLACE INTO sessions "
        "(sessionId,userId,username,displayName,role,expiresAt) "
        "VALUES (?,?,?,?,?,?)");
    q.bind(1, t.sessionId);
    q.bind(2, t.userId);
    q.bind(3, t.username);
    q.bind(4, t.displayName);
    q.bind(5, t.role);
    q.bind(6, (int64_t)t.expiresAt);
    q.exec();
}

std::optional<AuthToken> Database::findSession(const std::string& sessionId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    SQLite::Statement q(*db_, "SELECT * FROM sessions WHERE sessionId=?");
    q.bind(1, sessionId);
    if (!q.executeStep()) return std::nullopt;
    auto t = rowToToken(q);
    auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    if (t.expiresAt < now) {
        db_->exec("DELETE FROM sessions WHERE sessionId='" + sessionId + "'");
        return std::nullopt;
    }
    return t;
}

void Database::removeSession(const std::string& sessionId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    SQLite::Statement q(*db_, "DELETE FROM sessions WHERE sessionId=?");
    q.bind(1, sessionId);
    q.exec();
}

// ─────────────────────────────────────────────────────────────────────────────
//  CHAT ROOMS
// ─────────────────────────────────────────────────────────────────────────────
bool Database::createRoom(const ChatRoom& room)
{
    std::lock_guard<std::mutex> lock(mutex_);
    try {
        std::string typeStr =
            room.type == RoomType::GROUP  ? "GROUP"  :
            room.type == RoomType::DIRECT ? "DIRECT" : "PUBLIC";
        SQLite::Statement q(*db_,
            "INSERT INTO chat_rooms (roomId,name,type,creatorId,createdAt) "
            "VALUES (?,?,?,?,?)");
        q.bind(1, room.roomId);
        q.bind(2, room.name);
        q.bind(3, typeStr);
        q.bind(4, room.creatorId);
        q.bind(5, room.createdAt);
        q.exec();
        // add members
        for (auto& uid : room.memberIds) {
            SQLite::Statement m(*db_,
                "INSERT OR IGNORE INTO chat_members (roomId,userId) VALUES (?,?)");
            m.bind(1, room.roomId);
            m.bind(2, uid);
            m.exec();
        }
        return true;
    } catch (const SQLite::Exception& e) {
        std::cerr << "createRoom: " << e.what() << "\n";
        return false;
    }
}

std::optional<ChatRoom> Database::findRoom(const std::string& roomId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    SQLite::Statement q(*db_, "SELECT * FROM chat_rooms WHERE roomId=?");
    q.bind(1, roomId);
    if (!q.executeStep()) return std::nullopt;

    ChatRoom room;
    room.roomId    = q.getColumn("roomId").getText();
    room.name      = q.getColumn("name").getText();
    room.creatorId = q.getColumn("creatorId").getText();
    room.createdAt = q.getColumn("createdAt").getText();
    std::string t  = q.getColumn("type").getText();
    room.type = (t=="GROUP") ? RoomType::GROUP :
                (t=="DIRECT") ? RoomType::DIRECT : RoomType::PUBLIC;

    SQLite::Statement m(*db_,
        "SELECT userId FROM chat_members WHERE roomId=?");
    m.bind(1, roomId);
    while (m.executeStep())
        room.memberIds.push_back(m.getColumn(0).getText());

    return room;
}

bool Database::addMemberToRoom(const std::string& roomId, const std::string& userId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    SQLite::Statement q(*db_,
        "INSERT OR IGNORE INTO chat_members (roomId,userId) VALUES (?,?)");
    q.bind(1, roomId); q.bind(2, userId);
    q.exec();
    return true;
}

bool Database::isMember(const std::string& roomId, const std::string& userId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // check if PUBLIC first
    SQLite::Statement t(*db_, "SELECT type FROM chat_rooms WHERE roomId=?");
    t.bind(1, roomId);
    if (!t.executeStep()) return false;
    if (std::string(t.getColumn(0).getText()) == "PUBLIC") return true;

    SQLite::Statement q(*db_,
        "SELECT 1 FROM chat_members WHERE roomId=? AND userId=?");
    q.bind(1, roomId); q.bind(2, userId);
    return q.executeStep();
}

bool Database::addMessageToRoom(const std::string& roomId, const ChatMessage& msg)
{
    std::lock_guard<std::mutex> lock(mutex_);
    try {
        SQLite::Statement q(*db_,
            "INSERT INTO chat_messages "
            "(messageId,roomId,senderUserId,senderUsername,text,mediaUrl,timestamp) "
            "VALUES (?,?,?,?,?,?,?)");
        q.bind(1, msg.messageId);
        q.bind(2, roomId);
        q.bind(3, msg.senderUserId);
        q.bind(4, msg.senderUsername);
        q.bind(5, msg.text);
        q.bind(6, msg.mediaUrl);
        q.bind(7, msg.timestamp);
        q.exec();
        return true;
    } catch (const SQLite::Exception& e) {
        std::cerr << "addMessageToRoom: " << e.what() << "\n";
        return false;
    }
}

std::vector<ChatMessage> Database::getRoomHistory(const std::string& roomId, int limit)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ChatMessage> result;
    SQLite::Statement q(*db_,
        "SELECT * FROM chat_messages WHERE roomId=? "
        "ORDER BY timestamp DESC LIMIT ?");
    q.bind(1, roomId); q.bind(2, limit);
    while (q.executeStep()) result.push_back(rowToMessage(q));
    std::reverse(result.begin(), result.end());
    return result;
}

std::vector<ChatRoom> Database::getPublicRooms()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ChatRoom> result;
    SQLite::Statement q(*db_, "SELECT roomId FROM chat_rooms WHERE type='PUBLIC'");
    while (q.executeStep()) {
        auto r = findRoom(q.getColumn(0).getText());
        if (r) result.push_back(*r);
    }
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
//  MARKETPLACE
// ─────────────────────────────────────────────────────────────────────────────
bool Database::addListing(const Listing& l)
{
    std::lock_guard<std::mutex> lock(mutex_);
    try {
        SQLite::Statement q(*db_,
            "INSERT INTO listings "
            "(listingId,sellerUserId,sellerUsername,title,description,price,mediaUrl,status,createdAt) "
            "VALUES (?,?,?,?,?,?,?,?,?)");
        q.bind(1, l.listingId);
        q.bind(2, l.sellerUserId);
        q.bind(3, l.sellerUsername);
        q.bind(4, l.title);
        q.bind(5, l.description);
        q.bind(6, l.price);
        q.bind(7, l.mediaUrl);
        q.bind(8, "ACTIVE");
        q.bind(9, l.createdAt);
        q.exec();
        return true;
    } catch (const SQLite::Exception& e) {
        std::cerr << "addListing: " << e.what() << "\n";
        return false;
    }
}

std::optional<Listing> Database::findListing(const std::string& listingId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    SQLite::Statement q(*db_, "SELECT * FROM listings WHERE listingId=?");
    q.bind(1, listingId);
    if (q.executeStep()) return rowToListing(q);
    return std::nullopt;
}

std::vector<Listing> Database::searchListings(const std::string& query)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Listing> result;
    SQLite::Statement q(*db_,
        "SELECT * FROM listings WHERE status='ACTIVE' AND "
        "(title LIKE ? OR description LIKE ?)");
    std::string like = "%" + query + "%";
    q.bind(1, like); q.bind(2, like);
    while (q.executeStep()) result.push_back(rowToListing(q));
    return result;
}

std::vector<Listing> Database::getListingsByUser(const std::string& userId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Listing> result;
    SQLite::Statement q(*db_,
        "SELECT * FROM listings WHERE sellerUserId=? AND status!='DELETED'");
    q.bind(1, userId);
    while (q.executeStep()) result.push_back(rowToListing(q));
    return result;
}

bool Database::deleteListing(const std::string& listingId,
                              const std::string& requestingUserId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    SQLite::Statement q(*db_,
        "UPDATE listings SET status='DELETED' "
        "WHERE listingId=? AND sellerUserId=?");
    q.bind(1, listingId); q.bind(2, requestingUserId);
    q.exec();
    return db_->getChanges() > 0;
}

bool Database::markListingSold(const std::string& listingId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    SQLite::Statement q(*db_,
        "UPDATE listings SET status='SOLD' WHERE listingId=?");
    q.bind(1, listingId);
    q.exec();
    return db_->getChanges() > 0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  FORUM
// ─────────────────────────────────────────────────────────────────────────────
bool Database::addQuestion(const ForumQuestion& fq)
{
    std::lock_guard<std::mutex> lock(mutex_);
    try {
        SQLite::Statement q(*db_,
            "INSERT INTO forum_questions "
            "(questionId,authorUserId,authorUsername,title,text,upvotes,downvotes,timestamp) "
            "VALUES (?,?,?,?,?,0,0,?)");
        q.bind(1, fq.questionId);
        q.bind(2, fq.authorUserId);
        q.bind(3, fq.authorUsername);
        q.bind(4, fq.title);
        q.bind(5, fq.text);
        q.bind(6, fq.timestamp);
        q.exec();
        return true;
    } catch (const SQLite::Exception& e) {
        std::cerr << "addQuestion: " << e.what() << "\n";
        return false;
    }
}

std::optional<ForumQuestion> Database::findQuestion(const std::string& questionId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return loadQuestion(questionId);
}

ForumQuestion Database::loadQuestion(const std::string& questionId)
{
    SQLite::Statement q(*db_,
        "SELECT * FROM forum_questions WHERE questionId=?");
    q.bind(1, questionId);
    if (!q.executeStep()) return {};

    ForumQuestion fq;
    fq.questionId     = q.getColumn("questionId").getText();
    fq.authorUserId   = q.getColumn("authorUserId").getText();
    fq.authorUsername = q.getColumn("authorUsername").getText();
    fq.title          = q.getColumn("title").getText();
    fq.text           = q.getColumn("text").getText();
    fq.upvotes        = q.getColumn("upvotes").getInt();
    fq.downvotes      = q.getColumn("downvotes").getInt();
    fq.timestamp      = q.getColumn("timestamp").getText();

    SQLite::Statement a(*db_,
        "SELECT * FROM forum_answers WHERE questionId=? ORDER BY timestamp ASC");
    a.bind(1, questionId);
    while (a.executeStep()) {
        ForumAnswer fa;
        fa.answerId       = a.getColumn("answerId").getText();
        fa.questionId     = questionId;
        fa.authorUserId   = a.getColumn("authorUserId").getText();
        fa.authorUsername = a.getColumn("authorUsername").getText();
        fa.text           = a.getColumn("text").getText();
        fa.isFaq          = a.getColumn("isFaq").getInt() != 0;
        fa.upvotes        = a.getColumn("upvotes").getInt();
        fa.downvotes      = a.getColumn("downvotes").getInt();
        fa.timestamp      = a.getColumn("timestamp").getText();
        fq.answers.push_back(fa);
    }
    return fq;
}

bool Database::addAnswer(const std::string& questionId, const ForumAnswer& fa)
{
    std::lock_guard<std::mutex> lock(mutex_);
    try {
        SQLite::Statement q(*db_,
            "INSERT INTO forum_answers "
            "(answerId,questionId,authorUserId,authorUsername,text,isFaq,upvotes,downvotes,timestamp) "
            "VALUES (?,?,?,?,?,0,0,0,?)");
        q.bind(1, fa.answerId);
        q.bind(2, questionId);
        q.bind(3, fa.authorUserId);
        q.bind(4, fa.authorUsername);
        q.bind(5, fa.text);
        q.bind(6, fa.timestamp);
        q.exec();
        return true;
    } catch (const SQLite::Exception& e) {
        std::cerr << "addAnswer: " << e.what() << "\n";
        return false;
    }
}

bool Database::markAnswerFaq(const std::string& questionId,
                              const std::string& answerId,
                              const std::string& requestingUserId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // admin check would go here — for now just mark it
    SQLite::Statement q(*db_,
        "UPDATE forum_answers SET isFaq=1 WHERE answerId=? AND questionId=?");
    q.bind(1, answerId); q.bind(2, questionId);
    q.exec();
    return db_->getChanges() > 0;
}

bool Database::voteQuestion(const std::string& questionId,
                             const std::string& userId, bool upvote)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // check duplicate
    SQLite::Statement check(*db_,
        "SELECT voteType FROM forum_votes WHERE targetId=? AND userId=?");
    check.bind(1, questionId); check.bind(2, userId);
    if (check.executeStep()) return false; // already voted

    std::string vt = upvote ? "UP" : "DOWN";
    SQLite::Statement ins(*db_,
        "INSERT INTO forum_votes (targetId,userId,voteType) VALUES (?,?,?)");
    ins.bind(1, questionId); ins.bind(2, userId); ins.bind(3, vt);
    ins.exec();

    std::string col = upvote ? "upvotes" : "downvotes";
    db_->exec("UPDATE forum_questions SET " + col + "=" + col + "+1 "
              "WHERE questionId='" + questionId + "'");
    return true;
}

bool Database::voteAnswer(const std::string& questionId,
                           const std::string& answerId,
                           const std::string& userId, bool upvote)
{
    std::lock_guard<std::mutex> lock(mutex_);
    SQLite::Statement check(*db_,
        "SELECT voteType FROM forum_votes WHERE targetId=? AND userId=?");
    check.bind(1, answerId); check.bind(2, userId);
    if (check.executeStep()) return false;

    std::string vt = upvote ? "UP" : "DOWN";
    SQLite::Statement ins(*db_,
        "INSERT INTO forum_votes (targetId,userId,voteType) VALUES (?,?,?)");
    ins.bind(1, answerId); ins.bind(2, userId); ins.bind(3, vt);
    ins.exec();

    std::string col = upvote ? "upvotes" : "downvotes";
    db_->exec("UPDATE forum_answers SET " + col + "=" + col + "+1 "
              "WHERE answerId='" + answerId + "'");
    return true;
}

std::vector<ForumQuestion> Database::getAllQuestions()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ForumQuestion> result;
    SQLite::Statement q(*db_,
        "SELECT questionId FROM forum_questions ORDER BY timestamp DESC");
    while (q.executeStep())
        result.push_back(loadQuestion(q.getColumn(0).getText()));
    return result;
}

std::vector<ForumAnswer> Database::getFaqAnswers(const std::string& questionId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ForumAnswer> result;
    SQLite::Statement q(*db_,
        "SELECT * FROM forum_answers WHERE questionId=? AND isFaq=1");
    q.bind(1, questionId);
    while (q.executeStep()) {
        ForumAnswer fa;
        fa.answerId       = q.getColumn("answerId").getText();
        fa.questionId     = questionId;
        fa.authorUserId   = q.getColumn("authorUserId").getText();
        fa.authorUsername = q.getColumn("authorUsername").getText();
        fa.text           = q.getColumn("text").getText();
        fa.isFaq          = true;
        fa.upvotes        = q.getColumn("upvotes").getInt();
        fa.downvotes      = q.getColumn("downvotes").getInt();
        fa.timestamp      = q.getColumn("timestamp").getText();
        result.push_back(fa);
    }
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
//  FILES
// ─────────────────────────────────────────────────────────────────────────────
bool Database::addFile(const FileRecord& f)
{
    std::lock_guard<std::mutex> lock(mutex_);
    try {
        SQLite::Statement q(*db_,
            "INSERT INTO files "
            "(fileId,uploaderUserId,uploaderUsername,filename,url,fileSize,flagged,flagReason,uploadedAt) "
            "VALUES (?,?,?,?,?,?,0,'',?)");
        q.bind(1, f.fileId);
        q.bind(2, f.uploaderUserId);
        q.bind(3, f.uploaderUsername);
        q.bind(4, f.filename);
        q.bind(5, f.url);
        q.bind(6, f.fileSize);
        q.bind(7, f.uploadedAt);
        q.exec();
        return true;
    } catch (const SQLite::Exception& e) {
        std::cerr << "addFile: " << e.what() << "\n";
        return false;
    }
}

std::optional<FileRecord> Database::findFile(const std::string& fileId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    SQLite::Statement q(*db_, "SELECT * FROM files WHERE fileId=?");
    q.bind(1, fileId);
    if (!q.executeStep()) return std::nullopt;
    FileRecord f;
    f.fileId          = q.getColumn("fileId").getText();
    f.uploaderUserId  = q.getColumn("uploaderUserId").getText();
    f.uploaderUsername= q.getColumn("uploaderUsername").getText();
    f.filename        = q.getColumn("filename").getText();
    f.url             = q.getColumn("url").getText();
    f.fileSize        = q.getColumn("fileSize").getText();
    f.flagged         = q.getColumn("flagged").getInt() != 0;
    f.flagReason      = q.getColumn("flagReason").getText();
    f.uploadedAt      = q.getColumn("uploadedAt").getText();
    return f;
}

std::vector<FileRecord> Database::getAllFiles()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<FileRecord> result;
    SQLite::Statement q(*db_, "SELECT * FROM files WHERE flagged=0 ORDER BY uploadedAt DESC");
    while (q.executeStep()) {
        FileRecord f;
        f.fileId          = q.getColumn("fileId").getText();
        f.uploaderUserId  = q.getColumn("uploaderUserId").getText();
        f.uploaderUsername= q.getColumn("uploaderUsername").getText();
        f.filename        = q.getColumn("filename").getText();
        f.url             = q.getColumn("url").getText();
        f.fileSize        = q.getColumn("fileSize").getText();
        f.flagged         = false;
        f.uploadedAt      = q.getColumn("uploadedAt").getText();
        result.push_back(f);
    }
    return result;
}

bool Database::flagFile(const std::string& fileId, const std::string& reason)
{
    std::lock_guard<std::mutex> lock(mutex_);
    SQLite::Statement q(*db_,
        "UPDATE files SET flagged=1, flagReason=? WHERE fileId=?");
    q.bind(1, reason); q.bind(2, fileId);
    q.exec();
    return db_->getChanges() > 0;
}