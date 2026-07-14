#include "store/InMemoryStore.h"

InMemoryStore::InMemoryStore()
    : db_(std::make_unique<Database>("data/network.db"))
{}

// ── users ─────────────────────────────────────────────────────────────────────
bool InMemoryStore::addUser(const UserRecord& u)                              { return db_->addUser(u); }
std::optional<UserRecord> InMemoryStore::findUserById(const std::string& id)  { return db_->findUserById(id); }
std::optional<UserRecord> InMemoryStore::findUserByUsername(const std::string& u) { return db_->findUserByUsername(u); }
std::optional<UserRecord> InMemoryStore::findUserByEmail(const std::string& e)    { return db_->findUserByEmail(e); }
std::optional<UserRecord> InMemoryStore::findUserByUniversityId(const std::string& u) { return db_->findUserByUniversityId(u); }
bool InMemoryStore::updateUser(const std::string& id, const UserRecord& p)    { return db_->updateUser(id, p); }
std::vector<UserRecord> InMemoryStore::getAllUsers()                           { return db_->getAllUsers(); }

// ── sessions ──────────────────────────────────────────────────────────────────
void InMemoryStore::addSession(const AuthToken& t)                            { db_->addSession(t); }
std::optional<AuthToken> InMemoryStore::findSession(const std::string& id)    { return db_->findSession(id); }
void InMemoryStore::removeSession(const std::string& id)                      { db_->removeSession(id); }

// ── chat rooms ────────────────────────────────────────────────────────────────
bool InMemoryStore::createRoom(const ChatRoom& r)                             { return db_->createRoom(r); }
std::optional<ChatRoom> InMemoryStore::findRoom(const std::string& id)        { return db_->findRoom(id); }
bool InMemoryStore::addMemberToRoom(const std::string& r, const std::string& u) { return db_->addMemberToRoom(r, u); }
bool InMemoryStore::isMember(const std::string& r, const std::string& u)     { return db_->isMember(r, u); }
bool InMemoryStore::addMessageToRoom(const std::string& r, const ChatMessage& m) { return db_->addMessageToRoom(r, m); }
std::vector<ChatMessage> InMemoryStore::getRoomHistory(const std::string& id) { return db_->getRoomHistory(id); }
std::vector<ChatRoom> InMemoryStore::getPublicRooms()                         { return db_->getPublicRooms(); }
std::vector<ChatRoom> InMemoryStore::getRoomsForUser(const std::string& userId) { return db_->getRoomsForUser(userId); }

// ── marketplace ───────────────────────────────────────────────────────────────
bool InMemoryStore::addListing(const Listing& l)                              { return db_->addListing(l); }
std::optional<Listing> InMemoryStore::findListing(const std::string& id)      { return db_->findListing(id); }
std::vector<Listing> InMemoryStore::searchListings(const std::string& q)      { return db_->searchListings(q); }
std::vector<Listing> InMemoryStore::getListingsByUser(const std::string& id)  { return db_->getListingsByUser(id); }
bool InMemoryStore::deleteListing(const std::string& l, const std::string& u) { return db_->deleteListing(l, u); }
bool InMemoryStore::markListingSold(const std::string& id)                    { return db_->markListingSold(id); }

// ── forum ─────────────────────────────────────────────────────────────────────
bool InMemoryStore::addQuestion(const ForumQuestion& q)                       { return db_->addQuestion(q); }
std::optional<ForumQuestion> InMemoryStore::findQuestion(const std::string& id) { return db_->findQuestion(id); }
bool InMemoryStore::addAnswer(const std::string& qId, const ForumAnswer& a)   { return db_->addAnswer(qId, a); }
bool InMemoryStore::markAnswerFaq(const std::string& q, const std::string& a,
                                   const std::string& u)                      { return db_->markAnswerFaq(q, a, u); }
bool InMemoryStore::voteQuestion(const std::string& q, const std::string& u, bool up) { return db_->voteQuestion(q, u, up); }
bool InMemoryStore::voteAnswer(const std::string& q, const std::string& a,
                                const std::string& u, bool up)                { return db_->voteAnswer(q, a, u, up); }
std::vector<ForumQuestion> InMemoryStore::getAllQuestions()                   { return db_->getAllQuestions(); }
std::vector<ForumAnswer> InMemoryStore::getFaqAnswers(const std::string& id)  { return db_->getFaqAnswers(id); }

// ── files ─────────────────────────────────────────────────────────────────────
bool InMemoryStore::addFile(const FileRecord& f)                              { return db_->addFile(f); }
std::optional<FileRecord> InMemoryStore::findFile(const std::string& id)      { return db_->findFile(id); }
std::vector<FileRecord> InMemoryStore::getAllFiles()                           { return db_->getAllFiles(); }
bool InMemoryStore::flagFile(const std::string& id, const std::string& r)     { return db_->flagFile(id, r); }

// ── opportunities ────────────────────────────────────────────────────────────
bool InMemoryStore::addOpportunity(const Opportunity& o)                          { return db_->addOpportunity(o); }
std::optional<Opportunity> InMemoryStore::findOpportunity(const std::string& id)  { return db_->findOpportunity(id); }
std::vector<Opportunity> InMemoryStore::searchOpportunities(const std::string& q) { return db_->searchOpportunities(q); }
std::vector<Opportunity> InMemoryStore::getOpportunitiesByUser(const std::string& id) { return db_->getOpportunitiesByUser(id); }
bool InMemoryStore::deleteOpportunity(const std::string& o, const std::string& u) { return db_->deleteOpportunity(o, u); }
bool InMemoryStore::closeOpportunity(const std::string& id)                       { return db_->closeOpportunity(id); }