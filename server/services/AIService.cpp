#include "services/AIService.h"
#include <vector>
#include <random>
#include <sstream>

std::string AIService::suggestForumAnswer(const std::string& title, const std::string& text)
{
    std::vector<std::string> templates = {
        "Based on your question about \"" + title + "\", here are a few things worth checking first: make sure you have reviewed the relevant course materials, and consider posting your specific error message or code if this is a technical issue. A classmate or TA should be able to help further.",
        "Good question! For \"" + title + "\", it often helps to break the problem into smaller steps and verify each one works before moving to the next. If you are stuck on a specific part, share more details and someone can point you in the right direction.",
        "This is a common question. For \"" + title + "\", I would recommend checking the course syllabus or lecture notes first, since this may already be covered there. If not, the forum community here is a great place to get a human answer too."
    };

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, templates.size() - 1);
    return templates[dist(gen)];
}

std::string AIService::summarizeChat(const std::vector<std::string>& messages)
{
    if (messages.empty())
        return "There are no messages in this room yet to summarize.";

    std::ostringstream oss;
    oss << "Summary of the last " << messages.size() << " message(s) in this room: ";
    oss << "the conversation covered " << messages.size()
        << " exchange(s) between participants. ";
    oss << "Most recent topic: \"" << messages.back().substr(0, 80)
        << (messages.back().size() > 80 ? "..." : "") << "\"";

    return oss.str();
}
