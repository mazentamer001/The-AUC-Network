#pragma once
#include <string>
#include <vector>

struct ForumAnswer {
    std::string answerId;
    std::string questionId;
    std::string authorUserId;
    std::string authorUsername;
    std::string text;
    bool        isFaq = false;     // user can mark their own answer as FAQ
    std::string timestamp;
};

struct ForumQuestion {
    std::string              questionId;
    std::string              authorUserId;
    std::string              authorUsername;
    std::string              title;
    std::string              text;
    std::vector<ForumAnswer> answers;
    std::string              timestamp;
};