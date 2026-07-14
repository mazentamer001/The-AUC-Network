#pragma once
#include <SQLiteCpp/SQLiteCpp.h>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>
#include "models/UserRecord.h"
#include "models/ChatRoom.h"
#include "models/Listing.h"
#include "models/ForumPost.h"
#include "models/FileRecord.h"
#include "AuthToken.h"
#include "models/Opportunity.h" 

// ─────────────────────────────────────────────────────────────────────────────
//  Database
//  Single SQLite database backing all server data.
//  Same public interface as InMemoryStore — drop-in replacement.
//  Thread-safe via a single mutex.
// ─────────────────────────────────────────────────────────────────────────────
class Database
{
public:
    explicit Database(const std::string& path = "data/network.db");

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
    std::vector<ChatMessage> getRoomHistory(const std::string& roomId,
                                            int limit = 100);
    std::vector<ChatRoom>    getPublicRooms();
    std::vector<ChatRoom>    getRoomsForUser(const std::string& userId);

    // ── marketplace ───────────────────────────────────────────────────────
    bool                   addListing(const Listing& listing);
    std::optional<Listing> findListing(const std::string& listingId);
    std::vector<Listing>   searchListings(const std::string& query);
    std::vector<Listing>   getListingsByUser(const std::string& userId);
    bool                   deleteListing(const std::string& listingId,
                                         const std::string& requestingUserId);
    bool                   markListingSold(const std::string& listingId);


    // ── opportunities ─────────────────────────────────────────────────────
    bool                       addOpportunity(const Opportunity& opp);
    std::optional<Opportunity> findOpportunity(const std::string& opportunityId);
    std::vector<Opportunity>   searchOpportunities(const std::string& query);
    std::vector<Opportunity>   getOpportunitiesByUser(const std::string& userId);
    bool                       deleteOpportunity(const std::string& opportunityId,
                                                 const std::string& requestingUserId);
    bool                       closeOpportunity(const std::string& opportunityId);


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
    void createTables();
    UserRecord   rowToUser   (SQLite::Statement& q);
    AuthToken    rowToToken  (SQLite::Statement& q);
    ChatMessage  rowToMessage(SQLite::Statement& q);
    Listing      rowToListing(SQLite::Statement& q);
    Opportunity  rowToOpportunity(SQLite::Statement& q);

    std::unique_ptr<SQLite::Database> db_;
    std::mutex                        mutex_;

    bool addUser_nolock(const UserRecord&);
    std::optional<UserRecord> findUserById_nolock(const std::string&);
    std::optional<UserRecord> findUserByUsername_nolock(const std::string&);
    std::optional<UserRecord> findUserByEmail_nolock(const std::string&);
    std::optional<UserRecord> findUserByUniversityId_nolock(const std::string&);
    bool updateUser_nolock(const std::string&, const UserRecord&);
    std::vector<UserRecord> getAllUsers_nolock();

    void addSession_nolock(const AuthToken&);
    std::optional<AuthToken> findSession_nolock(const std::string&);
    void removeSession_nolock(const std::string&);

    bool createRoom_nolock(const ChatRoom&);
    std::optional<ChatRoom> findRoom_nolock(const std::string&);
    bool addMemberToRoom_nolock(const std::string&, const std::string&);
    bool isMember_nolock(const std::string&, const std::string&);
    bool addMessageToRoom_nolock(const std::string&, const ChatMessage&);
    std::vector<ChatMessage> getRoomHistory_nolock(const std::string&, int);
    std::vector<ChatRoom> getPublicRooms_nolock();
    std::vector<ChatRoom> getRoomsForUser_nolock(const std::string& userId);

    bool addListing_nolock(const Listing&);
    std::optional<Listing> findListing_nolock(const std::string&);
    std::vector<Listing> searchListings_nolock(const std::string&);
    std::vector<Listing> getListingsByUser_nolock(const std::string&);
    bool deleteListing_nolock(const std::string&, const std::string&);
    bool markListingSold_nolock(const std::string&);

    // private _nolock declarations, next to the listing ones
    bool addOpportunity_nolock(const Opportunity&);
    std::optional<Opportunity> findOpportunity_nolock(const std::string&);
    std::vector<Opportunity> searchOpportunities_nolock(const std::string&);
    std::vector<Opportunity> getOpportunitiesByUser_nolock(const std::string&);
    bool deleteOpportunity_nolock(const std::string&, const std::string&);
    bool closeOpportunity_nolock(const std::string&);

    bool addQuestion_nolock(const ForumQuestion&);
    std::optional<ForumQuestion> findQuestion_nolock(const std::string&);
    ForumQuestion loadQuestion_nolock(const std::string&); 
    bool addAnswer_nolock(const std::string&, const ForumAnswer&);
    bool markAnswerFaq_nolock(const std::string&, const std::string&, const std::string&);
    bool voteQuestion_nolock(const std::string&, const std::string&, bool);
    bool voteAnswer_nolock(const std::string&, const std::string&, const std::string&, bool);
    std::vector<ForumQuestion> getAllQuestions_nolock();
    std::vector<ForumAnswer> getFaqAnswers_nolock(const std::string&);

    bool addFile_nolock(const FileRecord&);
    std::optional<FileRecord> findFile_nolock(const std::string&);
    std::vector<FileRecord> getAllFiles_nolock();
    bool flagFile_nolock(const std::string&, const std::string&);
};