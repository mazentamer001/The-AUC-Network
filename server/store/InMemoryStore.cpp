#include "store/InMemoryStore.h"
#include <algorithm>
#include <chrono>

InMemoryStore::InMemoryStore()
    : db_(std::make_unique<Database>("auc_network.db"))
{}

//users — delegate to SQLite
bool InMemoryStore::addUser(const UserRecord& user)
{
    return db_->addUser(user);
}

std::optional<UserRecord> InMemoryStore::findUserById(const std::string& userId)
{
    return db_->findUserById(userId);
}

std::optional<UserRecord> InMemoryStore::findUserByUsername(const std::string& username)
{
    return db_->findUserByUsername(username);
}

std::optional<UserRecord> InMemoryStore::findUserByEmail(const std::string& email)
{
    return db_->findUserByEmail(email);
}

std::optional<UserRecord> InMemoryStore::findUserByUniversityId(const std::string& uniId)
{
    return db_->findUserByUniversityId(uniId);
}

bool InMemoryStore::updateUser(const std::string& userId, const UserRecord& patch)
{
    return db_->updateUser(userId, patch);
}

//sessions
void InMemoryStore::addSession(const AuthToken& token)
{
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_[token.sessionId] = token;
}

std::optional<AuthToken> InMemoryStore::findSession(const std::string& sessionId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = sessions_.find(sessionId);
    if (it == sessions_.end()) return std::nullopt;
    auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    if (it->second.expiresAt < now) { sessions_.erase(it); return std::nullopt; }
    return it->second;
}

void InMemoryStore::removeSession(const std::string& sessionId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.erase(sessionId);
}

//chat rooms
bool InMemoryStore::createRoom(const ChatRoom& room)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (rooms_.count(room.roomId)) return false;
    rooms_[room.roomId] = room;
    return true;
}

std::optional<ChatRoom> InMemoryStore::findRoom(const std::string& roomId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = rooms_.find(roomId);
    if (it == rooms_.end()) return std::nullopt;
    return it->second;
}

bool InMemoryStore::addMemberToRoom(const std::string& roomId, const std::string& userId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = rooms_.find(roomId);
    if (it == rooms_.end()) return false;
    auto& members = it->second.memberIds;
    if (std::find(members.begin(), members.end(), userId) == members.end())
        members.push_back(userId);
    return true;
}

bool InMemoryStore::isMember(const std::string& roomId, const std::string& userId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = rooms_.find(roomId);
    if (it == rooms_.end()) return false;
    if (it->second.type == RoomType::PUBLIC) return true;
    auto& members = it->second.memberIds;
    return std::find(members.begin(), members.end(), userId) != members.end();
}

bool InMemoryStore::addMessageToRoom(const std::string& roomId, const ChatMessage& msg)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = rooms_.find(roomId);
    if (it == rooms_.end()) return false;
    auto& history = it->second.history;
    history.push_back(msg);
    if (history.size() > 100)
        history.erase(history.begin());
    return true;
}

std::vector<ChatMessage> InMemoryStore::getRoomHistory(const std::string& roomId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = rooms_.find(roomId);
    if (it == rooms_.end()) return {};
    return it->second.history;
}

std::vector<ChatRoom> InMemoryStore::getPublicRooms()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ChatRoom> result;
    for (auto& [id, room] : rooms_)
        if (room.type == RoomType::PUBLIC)
            result.push_back(room);
    return result;
}

//marketplace
bool InMemoryStore::addListing(const Listing& listing)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (listings_.count(listing.listingId)) return false;
    listings_[listing.listingId] = listing;
    return true;
}

std::optional<Listing> InMemoryStore::findListing(const std::string& listingId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = listings_.find(listingId);
    if (it == listings_.end()) return std::nullopt;
    return it->second;
}

std::vector<Listing> InMemoryStore::searchListings(const std::string& query)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Listing> result;
    std::string q = query;
    std::transform(q.begin(), q.end(), q.begin(), ::tolower);
    for (auto& [id, listing] : listings_)
    {
        if (listing.status != ListingStatus::ACTIVE) continue;
        std::string title = listing.title;
        std::transform(title.begin(), title.end(), title.begin(), ::tolower);
        if (q.empty() || title.find(q) != std::string::npos)
            result.push_back(listing);
    }
    return result;
}

std::vector<Listing> InMemoryStore::getListingsByUser(const std::string& userId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Listing> result;
    for (auto& [id, listing] : listings_)
        if (listing.sellerUserId == userId && listing.status != ListingStatus::DELETED)
            result.push_back(listing);
    return result;
}

bool InMemoryStore::deleteListing(const std::string& listingId, const std::string& requestingUserId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = listings_.find(listingId);
    if (it == listings_.end()) return false;
    if (it->second.sellerUserId != requestingUserId) return false;
    it->second.status = ListingStatus::DELETED;
    return true;
}

bool InMemoryStore::markListingSold(const std::string& listingId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = listings_.find(listingId);
    if (it == listings_.end()) return false;
    it->second.status = ListingStatus::SOLD;
    return true;
}

//forum
bool InMemoryStore::addQuestion(const ForumQuestion& q)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (questions_.count(q.questionId)) return false;
    questions_[q.questionId] = q;
    return true;
}

std::optional<ForumQuestion> InMemoryStore::findQuestion(const std::string& questionId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = questions_.find(questionId);
    if (it == questions_.end()) return std::nullopt;
    return it->second;
}

bool InMemoryStore::addAnswer(const std::string& questionId, const ForumAnswer& answer)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = questions_.find(questionId);
    if (it == questions_.end()) return false;
    it->second.answers.push_back(answer);
    return true;
}

bool InMemoryStore::markAnswerFaq(const std::string& questionId, const std::string& answerId, const std::string& requestingUserId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = questions_.find(questionId);
    if (it == questions_.end()) return false;
    for (auto& a : it->second.answers)
    {
        if (a.answerId == answerId)
        {
            if (a.authorUserId != requestingUserId) return false;
            a.isFaq = true;
            return true;
        }
    }
    return false;
}

std::vector<ForumQuestion> InMemoryStore::getAllQuestions()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ForumQuestion> result;
    for (auto& [id, q] : questions_)
        result.push_back(q);
    return result;
}

std::vector<ForumAnswer> InMemoryStore::getFaqAnswers(const std::string& questionId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = questions_.find(questionId);
    if (it == questions_.end()) return {};
    std::vector<ForumAnswer> faqs;
    for (auto& a : it->second.answers)
        if (a.isFaq) faqs.push_back(a);
    return faqs;
}

//files
bool InMemoryStore::addFile(const FileRecord& file)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (files_.count(file.fileId)) return false;
    files_[file.fileId] = file;
    return true;
}

std::optional<FileRecord> InMemoryStore::findFile(const std::string& fileId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = files_.find(fileId);
    if (it == files_.end()) return std::nullopt;
    return it->second;
}

std::vector<FileRecord> InMemoryStore::getAllFiles()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<FileRecord> result;
    for (auto& [id, f] : files_)
        if (!f.flagged) result.push_back(f);
    return result;
}

bool InMemoryStore::flagFile(const std::string& fileId, const std::string& reason)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = files_.find(fileId);
    if (it == files_.end()) return false;
    it->second.flagged    = true;
    it->second.flagReason = reason;
    return true;
}

bool InMemoryStore::voteQuestion(const std::string& questionId, const std::string& userId, bool upvote)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = questions_.find(questionId);
    if (it == questions_.end()) return false;
    auto& q = it->second;

    if (upvote) {
        if (q.upvoters.count(userId))   return false;
        q.downvoters.erase(userId);
        if (q.downvotes > 0) q.downvotes--;
        q.upvoters.insert(userId);
        q.upvotes++;
    } else {
        if (q.downvoters.count(userId)) return false;
        q.upvoters.erase(userId);
        if (q.upvotes > 0) q.upvotes--;
        q.downvoters.insert(userId);
        q.downvotes++;
    }
    return true;
}

bool InMemoryStore::voteAnswer(const std::string& questionId, const std::string& answerId, const std::string& userId, bool upvote)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = questions_.find(questionId);
    if (it == questions_.end()) return false;
    for (auto& a : it->second.answers) {
        if (a.answerId == answerId) {
            if (upvote) {
                if (a.upvoters.count(userId))   return false;
                a.downvoters.erase(userId);
                if (a.downvotes > 0) a.downvotes--;
                a.upvoters.insert(userId);
                a.upvotes++;
            } else {
                if (a.downvoters.count(userId)) return false;
                a.upvoters.erase(userId);
                if (a.upvotes > 0) a.upvotes--;
                a.downvoters.insert(userId);
                a.downvotes++;
            }
            return true;
        }
    }
    return false;
}
