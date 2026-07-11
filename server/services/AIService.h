#pragma once
#include <string>

// ─────────────────────────────────────────────────────────────────────────────
//  AIService
//  Currently returns mocked responses. Once a real Anthropic API key is
//  available, only the body of suggestForumAnswer() needs to change to make
//  an actual HTTP request — nothing else in the codebase is affected.
// ─────────────────────────────────────────────────────────────────────────────
class AIService
{
public:
    std::string suggestForumAnswer(const std::string& title, const std::string& text);
};
