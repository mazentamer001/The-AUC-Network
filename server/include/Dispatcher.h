#pragma once
#include <memory>
#include "Message.h"

class Session;
class Server;

//-->forward-declare all services
class AuthService;
class RegistrationService;
class ProfileService;
class ChatService;
class MarketplaceService;
class OpportunityService;
class ForumService;
class FileStorageService;


//  Dispatcher
//  Responsibilities:
//    - Receive every deserialized Message from Session::do_read()
//    - Validate that the sender is authenticated (except AUTH_LOGIN / AUTH_REGISTER)
//    - Route to the correct service based on msg.type
//    - Send back an ERROR message for unknown / unauthorized requests
//
//  Knows nothing about sockets or bytes — pure message routing.



class Dispatcher
{
public:
    Dispatcher(AuthService&         auth,
               RegistrationService& registration,
               ProfileService&      profile,
               ChatService&         chat,
               MarketplaceService&  marketplace,
               OpportunityService&  opportunities,
               ForumService&        forum,
               FileStorageService&  fileStorage);

    void setServer(Server& server) { server_ = &server; }
    void dispatch(const Message& msg, std::shared_ptr<Session> sender);

private:
    bool requireAuth(const Message& msg, std::shared_ptr<Session> sender);
    void sendError(const std::string& reason, std::shared_ptr<Session> sender);
    Server* server_ = nullptr;
    AuthService& auth_;
    RegistrationService& registration_;
    ProfileService& profile_;
    ChatService& chat_;
    MarketplaceService&  marketplace_;
    OpportunityService&  opportunities_;
    ForumService& forum_;
    FileStorageService& fileStorage_;
};
