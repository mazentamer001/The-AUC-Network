#pragma once
#include <string>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
//  AIService
//  Currently returns mocked responses. Once a real Anthropic API key is
//  available, only the body of these functions needs to change to make
//  an actual HTTP request — nothing else in the codebase is affected.
// ─────────────────────────────────────────────────────────────────────────────
class AIService
{
public:
    std::string suggestForumAnswer(const std::string& title, const std::string& text);
    std::string summarizeChat(const std::vector<std::string>& messages);
};
