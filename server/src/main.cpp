#include <boost/asio.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

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
#include "services/AIService.h"

// Loads simple KEY=VALUE lines from a .env file into the process environment.
// Existing environment variables are never overwritten — if GROQ_API_KEY is
// already set (e.g. via systemd EnvironmentFile or your shell), the .env
// file is just a fallback, not a hijack.
// Lines starting with '#' and blank lines are skipped.
static void loadDotEnv(const std::string& path)
{
    std::ifstream file(path);
    if (!file) {
        std::cerr << "[env] No " << path << " found — relying on real "
                     "environment variables only.\n";
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        // trim trailing \r in case the file was saved with CRLF line endings
        if (!line.empty() && line.back() == '\r') line.pop_back();

        if (line.empty() || line[0] == '#') continue;

        auto eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);

        if (key.empty()) continue;

        // don't clobber a real environment variable that's already set
        setenv(key.c_str(), value.c_str(), /*overwrite=*/0);
    }

    std::cout << "[env] Loaded " << path << "\n";
}

int main()
{
    // must happen before AIService (or anything else reading getenv) is constructed
    loadDotEnv(".env");

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
        AIService            ai(io);

        // ── dispatcher ────────────────────────────────────────────────────
        Dispatcher dispatcher(auth, registration, profile,
                              chat, marketplace, forum, fileStorage,
                              opportunity, ai);

        // ── server ────────────────────────────────────────────────────────
        Server server(io, 12345, dispatcher);

        // ── back-fill server references ───────────────────────────────────
        auth.setServer(server);
        chat.setServer(server);
        marketplace.setServer(server);
        opportunity.setServer(server);
        forum.setServer(server);
        fileStorage.setServer(server);
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