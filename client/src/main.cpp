#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <thread>

using boost::asio::ip::tcp;
using json = nlohmann::json;

tcp::socket* g_socket = nullptr;
std::string  g_token;

void printResp(const std::string& raw) {
    try {
        json j = json::parse(raw);
        std::string type = j.value("type","?");
        std::string text = j.value("text","");
        std::cout << "\n[SERVER] " << type;
        if (!text.empty())                       std::cout << " | " << text;
        if (!j.value("token","").empty())        std::cout << "\n  token: " << j["token"].get<std::string>();
        if (!j.value("role","").empty())         std::cout << " | role: " << j["role"].get<std::string>();
        if (!j.value("displayName","").empty())  std::cout << " | name: " << j["displayName"].get<std::string>();
        if (!j.value("title","").empty())        std::cout << " | title: " << j["title"].get<std::string>();
        if (!j.value("price","").empty())        std::cout << " | price: " << j["price"].get<std::string>();
        if (!j.value("filename","").empty())     std::cout << " | file: " << j["filename"].get<std::string>();
        if (!j.value("mediaUrl","").empty())     std::cout << " | url: " << j["mediaUrl"].get<std::string>();
        if (!j.value("parentId","").empty())     std::cout << " | id: " << j["parentId"].get<std::string>();
        if (!j.value("roomId","").empty())       std::cout << " | room: " << j["roomId"].get<std::string>();
        std::cout << "\n";
        // auto-save token
        if (!j.value("token","").empty()) {
            g_token = j["token"].get<std::string>();
            std::cout << "  [token saved]\n";
        }
    } catch(...) { std::cout << "\n[SERVER] " << raw << "\n"; }
}

void send(const json& msg) {
    std::string p = msg.dump() + "\n";
    boost::asio::write(*g_socket, boost::asio::buffer(p));
}


//this is a test run via the terminal to test all implemented features
int main() {
    boost::asio::io_context io;
    tcp::resolver resolver(io);
    tcp::socket socket(io);
    g_socket = &socket;
    boost::asio::connect(socket, resolver.resolve("127.0.0.1","12345"));
    std::cout << "Connected! Commands:\n"
              << "  register | login | logout\n"
              << "  profile-get | profile-edit\n"
              << "  room-create | room-join | room-leave\n"
              << "  chat | dm\n"
              << "  market-post | market-search | market-buy | market-delete\n"
              << "  forum-ask | forum-answer | forum-faq | forum-list | forum-get\n"
              << "  file-upload | file-list | file-report\n"
              << "  quit\n\n";

    // background reader
    boost::asio::streambuf buf;
    std::thread reader([&](){
        try {
            while(true) {
                boost::asio::read_until(socket, buf, '\n');
                std::istream is(&buf);
                std::string line; std::getline(is, line);
                printResp(line);
                std::cout << "> " << std::flush;
            }
        } catch(...) { std::cout << "[disconnected]\n"; }
    });

    std::string cmd;
    while(true) {
        std::cout << "> "; std::getline(std::cin, cmd);
        if (cmd.empty()) continue;
        if (cmd == "quit") break;

        auto ask = [](const std::string& q) {
            std::string v; std::cout << q; std::getline(std::cin, v); return v;
        };

        if (cmd == "register") {
            send({{"type","AUTH_REGISTER"},
                  {"username",    ask("username     : ")},
                  {"displayName", ask("display name : ")},
                  {"email",       ask("email        : ")},
                  {"password",    ask("password     : ")},
                  {"universityId",ask("university ID: ")},
                  {"bio",         ask("bio          : ")}});

        } else if (cmd == "login") {
            send({{"type","AUTH_LOGIN"},
                  {"username", ask("username/email: ")},
                  {"password", ask("password      : ")}});

        } else if (cmd == "logout") {
            send({{"type","AUTH_LOGOUT"},{"token",g_token}});
            g_token.clear();

        } else if (cmd == "profile-get") {
            send({{"type","PROFILE_GET"},{"token",g_token},
                  {"recipientId", ask("userId (blank=own): ")}});

        } else if (cmd == "profile-edit") {
            send({{"type","PROFILE_EDIT"},{"token",g_token},
                  {"displayName", ask("new display name: ")},
                  {"bio",         ask("new bio         : ")},
                  {"profilePicUrl",ask("pic url         : ")}});

        } else if (cmd == "room-create") {
            send({{"type","CHAT_CREATE"},{"token",g_token},
                  {"roomId", ask("room id  : ")},
                  {"text",   ask("room name: ")},
                  {"role",   ask("type (PUBLIC/GROUP): ")},
                  {"sender", {{"username","me"}}}});

        } else if (cmd == "room-join") {
            send({{"type","JOIN"},{"token",g_token},
                  {"roomId", ask("room id: ")},
                  {"sender", {{"username","me"}}}});

        } else if (cmd == "room-leave") {
            send({{"type","LEAVE"},{"token",g_token},
                  {"roomId", ask("room id: ")},
                  {"sender", {{"username","me"}}}});

        } else if (cmd == "chat") {
            send({{"type","CHAT_PUBLIC"},{"token",g_token},
                  {"roomId", ask("room id: ")},
                  {"text",   ask("message: ")},
                  {"sender", {{"username","me"}}}});

        } else if (cmd == "dm") {
            send({{"type","CHAT_PRIVATE"},{"token",g_token},
                  {"recipientId", ask("recipient userId: ")},
                  {"text",        ask("message         : ")},
                  {"sender",      {{"username","me"}}}});

        } else if (cmd == "market-post") {
            send({{"type","MARKET_POST"},{"token",g_token},
                  {"title",   ask("title      : ")},
                  {"price",   ask("price      : ")},
                  {"text",    ask("description: ")},
                  {"mediaUrl",ask("photo url  : ")},
                  {"sender",  {{"username","me"}}}});

        } else if (cmd == "market-search") {
            send({{"type","MARKET_SEARCH"},{"token",g_token},
                  {"text",   ask("search (blank=all): ")},
                  {"sender", {{"username","me"}}}});

        } else if (cmd == "market-buy") {
            send({{"type","MARKET_INQUIRY"},{"token",g_token},
                  {"parentId", ask("listing id : ")},
                  {"text",     ask("message    : ")},
                  {"sender",   {{"username","me"}}}});

        } else if (cmd == "market-delete") {
            send({{"type","MARKET_DELETE"},{"token",g_token},
                  {"parentId", ask("listing id: ")},
                  {"sender",   {{"username","me"}}}});

        } else if (cmd == "forum-ask") {
            send({{"type","QA_QUESTION"},{"token",g_token},
                  {"title",  ask("title   : ")},
                  {"text",   ask("details : ")},
                  {"sender", {{"username","me"}}}});

        } else if (cmd == "forum-answer") {
            send({{"type","QA_ANSWER"},{"token",g_token},
                  {"parentId", ask("question id: ")},
                  {"text",     ask("answer     : ")},
                  {"sender",   {{"username","me"}}}});

        } else if (cmd == "forum-faq") {
            send({{"type","QA_FAQ"},{"token",g_token},
                  {"parentId", ask("question id: ")},
                  {"filename", ask("answer id  : ")},
                  {"sender",   {{"username","me"}}}});

        } else if (cmd == "forum-list") {
            send({{"type","QA_GET_ALL"},{"token",g_token},
                  {"sender",{{"username","me"}}}});

        } else if (cmd == "forum-get") {
            send({{"type","QA_GET_ONE"},{"token",g_token},
                  {"parentId", ask("question id: ")},
                  {"sender",   {{"username","me"}}}});

        } else if (cmd == "file-upload") {
            send({{"type","MATERIAL_UPLOAD"},{"token",g_token},
                  {"filename", ask("filename   : ")},
                  {"text",     ask("content    : ")},
                  {"sender",   {{"username","me"}}}});

        } else if (cmd == "file-list") {
            send({{"type","MATERIAL_LIST"},{"token",g_token},
                  {"sender",{{"username","me"}}}});

        } else if (cmd == "file-report") {
            send({{"type","MATERIAL_REPORT"},{"token",g_token},
                  {"parentId", ask("file id: ")},
                  {"text",     ask("reason : ")},
                  {"sender",   {{"username","me"}}}});

        } else {
            std::cout << "Unknown command\n";
        }
    }

    socket.close();
    reader.detach();
    return 0;
}