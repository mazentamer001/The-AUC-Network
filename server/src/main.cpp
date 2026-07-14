#include <boost/asio.hpp>
#include <iostream>

#include "Server.h"
#include "Dispatcher.h"
#include "store/InMemoryStore.h"
#include "services/AuthService.h"
#include "services/RegistrationService.h"
#include "services/ProfileService.h"
#include "services/ChatService.h"
#include "services/MarketplaceService.h"
#include "services/OpportunityService.h"
#include "services/ForumService.h"
#include "services/FileStorageService.h"

int main()
{
    try
    {
        boost::asio::io_context io;

        InMemoryStore db;

        // ── services ──────────────────────────────────────────────────────
        AuthService         auth(db);
        RegistrationService registration(db);
        ProfileService      profile(db);
        ChatService         chat(db);
        MarketplaceService  marketplace(db);
        OpportunityService  opportunity(db);
        ForumService        forum(db);
        FileStorageService  fileStorage(db);

        // ── dispatcher ────────────────────────────────────────────────────
        Dispatcher dispatcher(auth, registration, profile,
                              chat, marketplace, forum, fileStorage,
                              opportunity);

        // ── server ────────────────────────────────────────────────────────
        Server server(io, 12345, dispatcher);

        // ── back-fill server references ───────────────────────────────────
        auth.setServer(server);
        chat.setServer(server);
        marketplace.setServer(server);
        opportunity.setServer(server);
        dispatcher.setServer(server);

        std::cout << "Server ready on port 12345\n";
        io.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}