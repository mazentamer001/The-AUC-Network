#pragma once
#include <string>

enum class OpportunityType { CLUB, COMPETITION, JOB };
enum class OpportunityStatus { OPEN, CLOSED, DELETED };

struct Opportunity {
    std::string opportunityId;
    std::string posterUserId;
    std::string posterUsername;
    OpportunityType type = OpportunityType::CLUB;
    std::string title;
    std::string description;
    std::string organization;
    std::string deadline;
    std::string contactInfo;
    OpportunityStatus status = OpportunityStatus::OPEN;
    std::string createdAt;
};
