#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <string>
#include "Message.h"

class Session;

// Summarizes a chat room's message log via Groq's chat completions API.
// The API key lives only here, read once from the GROQ_API_KEY environment
// variable — it never reaches any client.
//
// The HTTPS call is a blocking Beast/OpenSSL request, so it always runs on a
// detached worker thread, never on the io_context's thread (the server is
// single-threaded via one io.run() call, so blocking that thread would stall
// every other session). The result is handed back via boost::asio::post so
// Session::send() is still only ever called from the io_context thread.
class AIService
{
public:
    explicit AIService(boost::asio::io_context& io);

    void handleSummarize(const Message& msg, std::shared_ptr<Session> sender);

private:
    std::string callGroq(const std::string& roomLabel, const std::string& chatLog) const;

    boost::asio::io_context& io_;
    std::string apiKey_;
};