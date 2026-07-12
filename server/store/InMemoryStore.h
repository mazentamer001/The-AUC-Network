#pragma once
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "models/UserRecord.h"
#include "models/ChatRoom.h"
#include "models/Listing.h"
#include "models/ForumPost.h"
#include "models/FileRecord.h"
#include "AuthToken.h"
#include "store/Database.h"

// ─────────────────────────────────────────────────────────────────────────────
//  InMemoryStore — now a thin wrapper around Database (SQLite)
//  All data persists to disk. No in-memory maps.
//  Services use this class unchanged — only the backing store changed.
// ─────────────────────────────────────────────────────────────────────────────
class InMemoryStore
{
public:
    InMemoryStore();

    // ── users ─────────────────────────────────────────────────────────────
    bool                      addUser(const UserRecord& user);
    std::optional<UserRecord> findUserById(const std::string& userId);
    std::optional<UserRecord> findUserByUsername(const std::string& username);
    std::optional<UserRecord> findUserByEmail(const std::string& email);
    std::optional<UserRecord> findUserByUniversityId(const std::string& uniId);
    bool                      updateUser(const std::string& userId, const UserRecord& patch);
    std::vector<UserRecord>   getAllUsers();

    // ── sessions ──────────────────────────────────────────────────────────
    void                     addSession(const AuthToken& token);
    std::optional<AuthToken> findSession(const std::string& sessionId);
    void                     removeSession(const std::string& sessionId);

    // ── chat rooms ────────────────────────────────────────────────────────
    bool                     createRoom(const ChatRoom& room);
    std::optional<ChatRoom>  findRoom(const std::string& roomId);
    bool                     addMemberToRoom(const std::string& roomId,
                                             const std::string& userId);
    bool                     isMember(const std::string& roomId,
                                      const std::string& userId);
    bool                     addMessageToRoom(const std::string& roomId,
                                              const ChatMessage& msg);
    std::vector<ChatMessage> getRoomHistory(const std::string& roomId);
    std::vector<ChatRoom>    getPublicRooms();

    // ── marketplace ───────────────────────────────────────────────────────
    bool                   addListing(const Listing& listing);
    std::optional<Listing> findListing(const std::string& listingId);
    std::vector<Listing>   searchListings(const std::string& query);
    std::vector<Listing>   getListingsByUser(const std::string& userId);
    bool                   deleteListing(const std::string& listingId,
                                         const std::string& requestingUserId);
    bool                   markListingSold(const std::string& listingId);

    // ── forum ─────────────────────────────────────────────────────────────
    bool                         addQuestion(const ForumQuestion& q);
    std::optional<ForumQuestion> findQuestion(const std::string& questionId);
    bool                         addAnswer(const std::string& questionId,
                                           const ForumAnswer& answer);
    bool                         markAnswerFaq(const std::string& questionId,
                                               const std::string& answerId,
                                               const std::string& requestingUserId);
    bool                         voteQuestion(const std::string& questionId,
                                              const std::string& userId, bool upvote);
    bool                         voteAnswer(const std::string& questionId,
                                            const std::string& answerId,
                                            const std::string& userId, bool upvote);
    std::vector<ForumQuestion>   getAllQuestions();
    std::vector<ForumAnswer>     getFaqAnswers(const std::string& questionId);

    // ── files ─────────────────────────────────────────────────────────────
    bool                      addFile(const FileRecord& file);
    std::optional<FileRecord> findFile(const std::string& fileId);
    std::vector<FileRecord>   getAllFiles();
    bool                      flagFile(const std::string& fileId,
                                       const std::string& reason);

private:
    std::unique_ptr<Database> db_;
};