#pragma once
#include <string>
#include <vector>
#include <set>

struct ForumAnswer {
    std::string answerId;
    std::string questionId;
    std::string authorUserId;
    std::string authorUsername;
    std::string text;
    bool isFaq = false;
    int upvotes = 0;
    int downvotes = 0;
    std::set<std::string> upvoters;    //userIds who upvoted
    std::set<std::string> downvoters;  //userIds who downvoted
    std::string timestamp;
};

struct ForumQuestion {
    std::string questionId;
    std::string authorUserId;
    std::string authorUsername;
    std::string title;
    std::string text;
    int upvotes = 0;
    int downvotes = 0;
    std::set<std::string> upvoters;
    std::set<std::string> downvoters;
    std::vector<ForumAnswer> answers;
    std::string timestamp;
};