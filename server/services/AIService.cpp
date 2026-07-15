#include "AIService.h"
#include "Session.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>

#include <nlohmann/json.hpp>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <thread>

namespace beast = boost::beast;
namespace http  = beast::http;
using tcp       = boost::asio::ip::tcp;
using json      = nlohmann::json;

AIService::AIService(boost::asio::io_context& io) : io_(io)
{
    if (const char* key = std::getenv("GROQ_API_KEY"))
        apiKey_ = key;

    if (apiKey_.empty())
        std::cerr << "[AIService] Warning: GROQ_API_KEY not set — "
                     "summarize requests will fail until it is.\n";
}

void AIService::handleSummarize(const Message& msg, std::shared_ptr<Session> sender)
{
    if (msg.text.empty()) {
        Message err;
        err.type   = MessageType::ERROR;
        err.roomId = msg.roomId;
        err.text   = "Nothing to summarize yet.";
        boost::asio::post(io_, [sender, err]() { sender->send(err); });
        return;
    }

    if (apiKey_.empty()) {
        Message err;
        err.type   = MessageType::ERROR;
        err.roomId = msg.roomId;
        err.text   = "AI summarizer isn't configured on the server (missing API key).";
        boost::asio::post(io_, [sender, err]() { sender->send(err); });
        return;
    }

    // copy everything the worker thread needs — msg/sender must not be touched
    // off the io_context thread beyond this point
    const std::string roomId   = msg.roomId;
    const std::string roomLabel = msg.roomId; // client can pass a friendlier label via msg.title later if wanted
    const std::string chatLog  = msg.text;

    std::thread([this, sender, roomId, roomLabel, chatLog]() {
        Message response;
        response.roomId = roomId;
        try {
            response.type = MessageType::AI_SUMMARIZE_RESPONSE;
            response.text = callGroq(roomLabel, chatLog);
        } catch (const std::exception& e) {
            response.type = MessageType::ERROR;
            response.text = std::string("Summarize failed: ") + e.what();
        }
        boost::asio::post(io_, [sender, response]() { sender->send(response); });
    }).detach();
}

std::string AIService::callGroq(const std::string& roomLabel, const std::string& chatLog) const
{
    boost::asio::io_context ioc;
    boost::asio::ssl::context ctx(boost::asio::ssl::context::tlsv12_client);
    ctx.set_default_verify_paths();

    tcp::resolver resolver(ioc);
    beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);

    if (!SSL_set_tlsext_host_name(stream.native_handle(), "api.groq.com"))
        throw std::runtime_error("SSL SNI setup failed");

    auto const results = resolver.resolve("api.groq.com", "443");
    beast::get_lowest_layer(stream).connect(results);
    stream.handshake(boost::asio::ssl::stream_base::client);

    json body;
    body["model"]       = "llama-3.3-70b-versatile"; // check console.groq.com/docs/models if retired
    body["temperature"] = 0.3;
    body["max_tokens"]  = 400;
    body["messages"] = json::array({
        { {"role", "system"}, {"content",
            "You summarize group chat conversations. Be concise (5-8 bullet points max), "
            "focus on decisions, questions, and action items, and use participants' actual "
            "names when referring to them."} },
        { {"role", "user"}, {"content",
            "Summarize the conversation in the room \"" + roomLabel + "\":\n\n" + chatLog} }
    });

    http::request<http::string_body> req{http::verb::post, "/openai/v1/chat/completions", 11};
    req.set(http::field::host, "api.groq.com");
    req.set(http::field::authorization, "Bearer " + apiKey_);
    req.set(http::field::content_type, "application/json");
    req.set(http::field::user_agent, "AUCNetworking-Server");
    req.body() = body.dump();
    req.prepare_payload();

    http::write(stream, req);

    beast::flat_buffer buffer;
    http::response<http::string_body> res;
    http::read(stream, buffer, res);

    beast::error_code ec;
    stream.shutdown(ec); // Groq (like most servers) RSTs rather than closing cleanly; ignore

    if (res.result() != http::status::ok)
        throw std::runtime_error("Groq API returned HTTP " +
            std::to_string(res.result_int()) + ": " + res.body());

    json resJson = json::parse(res.body());
    auto choices = resJson.value("choices", json::array());
    if (choices.empty())
        throw std::runtime_error("Unexpected response from Groq API");

    std::string content = choices.at(0).at("message").at("content").get<std::string>();
    if (content.empty())
        throw std::runtime_error("Empty summary returned");

    return content;
}