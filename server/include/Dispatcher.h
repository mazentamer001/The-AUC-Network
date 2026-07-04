#pragma once
#include <memory>
#include "Message.h"

class Session;
class Server;

// ── forward-declare all services ───────────────────────────────────────────
class AuthService;
class RegistrationService;
class ProfileService;
class ChatService;
class MarketplaceService;
class ForumService;
class FileStorageService;

// ─────────────────────────────────────────────────────────────────────────────
//  Dispatcher
//  Responsibilities:
//    - Receive every deserialized Message from Session::do_read()
//    - Validate that the sender is authenticated (except AUTH_LOGIN / AUTH_REGISTER)
//    - Route to the correct service based on msg.type
//    - Send back an ERROR message for unknown / unauthorized requests
//
//  Knows nothing about sockets or bytes — pure message routing.
// ─────────────────────────────────────────────────────────────────────────────
class Dispatcher
{
public:
    // Services injected at construction. Server back-filled via setServer()
    // after Server is constructed (breaks the Server↔Dispatcher cycle).
    Dispatcher(AuthService&         auth,
               RegistrationService& registration,
               ProfileService&      profile,
               ChatService&         chat,
               MarketplaceService&  marketplace,
               ForumService&        forum,
               FileStorageService&  fileStorage);

    void setServer(Server& server) { server_ = &server; }

    // Called by Session after every successful deserialize
    void dispatch(const Message& msg, std::shared_ptr<Session> sender);

private:
    // ── auth check helper ──────────────────────────────────────────────────
    // Returns true if msg.token resolves to a valid session.
    // Sends an ERROR back and returns false if not.
    bool requireAuth(const Message& msg, std::shared_ptr<Session> sender);

    // ── send helpers ───────────────────────────────────────────────────────
    void sendError(const std::string& reason, std::shared_ptr<Session> sender);

    // ── service references (non-owning) ────────────────────────────────────
    Server*              server_ = nullptr;
    AuthService&         auth_;
    RegistrationService& registration_;
    ProfileService&      profile_;
    ChatService&         chat_;
    MarketplaceService&  marketplace_;
    ForumService&        forum_;
    FileStorageService&  fileStorage_;
};