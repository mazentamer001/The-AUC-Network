#pragma once
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include "models/UserRecord.h"
#include "models/ChatRoom.h"
#include "models/Listing.h"
#include "models/Opportunity.h"
#include "models/ForumPost.h"
#include "models/FileRecord.h"
#include "AuthToken.h"
#include "store/Database.h"

//users now persist to SQLite via Database; everything else stays in-memory
class InMemoryStore
{
public:
    InMemoryStore();

    //users — now backed by SQLite
    bool addUser(const UserRecord& user);
    std::optional<UserRecord> findUserById(const std::string& userId);
    std::optional<UserRecord> findUserByUsername(const std::string& username);
    std::optional<UserRecord> findUserByEmail(const std::string& email);
    std::optional<UserRecord> findUserByUniversityId(const std::string& uniId);
    bool updateUser(const std::string& userId, const UserRecord& patch);

    //sessions
    void addSession(const AuthToken& token);
    std::optional<AuthToken> findSession(const std::string& sessionId);
    void removeSession(const std::string& sessionId);

    //chat rooms
    bool createRoom(const ChatRoom& room);
    std::optional<ChatRoom> findRoom(const std::string& roomId);
    bool addMemberToRoom(const std::string& roomId, const std::string& userId);
    bool isMember(const std::string& roomId, const std::string& userId);
    bool addMessageToRoom(const std::string& roomId, const ChatMessage& msg);
    std::vector<ChatMessage> getRoomHistory(const std::string& roomId);
    std::vector<ChatRoom> getPublicRooms();

    // marketplace
    bool addListing(const Listing& listing);
    std::optional<Listing> findListing(const std::string& listingId);
    std::vector<Listing> searchListings(const std::string& query);
    std::vector<Listing> getListingsByUser(const std::string& userId);
    bool deleteListing(const std::string& listingId, const std::string& requestingUserId);
    bool markListingSold(const std::string& listingId);

    // opportunities
    bool addOpportunity(const Opportunity& opp);
    std::optional<Opportunity> findOpportunity(const std::string& opportunityId);
    std::vector<Opportunity> searchOpportunities(const std::string& query);
    std::vector<Opportunity> getOpportunitiesByUser(const std::string& userId);
    bool deleteOpportunity(const std::string& opportunityId, const std::string& requestingUserId);
    bool markOpportunityClosed(const std::string& opportunityId);

    // ── forum ─────────────────────────────────────────────────────────────
    bool addQuestion(const ForumQuestion& q);
    std::optional<ForumQuestion> findQuestion(const std::string& questionId);
    bool addAnswer(const std::string& questionId, const ForumAnswer& answer);
    bool markAnswerFaq(const std::string& questionId, const std::string& answerId, const std::string& requestingUserId);
    std::vector<ForumQuestion> getAllQuestions();
    std::vector<ForumAnswer> getFaqAnswers(const std::string& questionId);
    bool voteQuestion(const std::string& questionId, const std::string& userId, bool upvote);
    bool voteAnswer(const std::string& questionId, const std::string& answerId, const std::string& userId, bool upvote);

    //files
    bool addFile(const FileRecord& file);
    std::optional<FileRecord> findFile(const std::string& fileId);
    std::vector<FileRecord> getAllFiles();
    bool flagFile(const std::string& fileId, const std::string& reason);

private:
    std::mutex mutex_;
    std::unique_ptr<Database> db_;

    //sessions
    std::unordered_map<std::string, AuthToken> sessions_;

    //chat
    std::unordered_map<std::string, ChatRoom> rooms_;

    //marketplace
    std::unordered_map<std::string, Listing> listings_;

    //opportunities
    std::unordered_map<std::string, Opportunity> opportunities_;

    //forum
    std::unordered_map<std::string, ForumQuestion> questions_;

    //files
    std::unordered_map<std::string, FileRecord> files_;
};
