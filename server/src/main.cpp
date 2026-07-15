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
#include "services/ForumService.h"
#include "services/FileStorageService.h"
#include "services/AIService.h"

int main()
{
    try
    {
        boost::asio::io_context io;

        InMemoryStore store;
        AIService ai;

        AuthService         auth(store);
        RegistrationService registration(store);
        ProfileService      profile(store);
        ChatService         chat(store, ai);
        MarketplaceService  marketplace(store);
        ForumService        forum(store, ai);
        FileStorageService  fileStorage(store);

        Dispatcher dispatcher(auth, registration, profile,
                              chat, marketplace, forum, fileStorage);

        Server server(io, 12345, dispatcher);

        auth.setServer(server);
        chat.setServer(server);
        marketplace.setServer(server);
        dispatcher.setServer(server);
        fileStorage.setServer(server);
        forum.setServer(server);

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
