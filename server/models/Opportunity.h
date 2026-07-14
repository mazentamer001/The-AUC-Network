#pragma once
#include <string>

enum class OpportunityStatus { ACTIVE, CLOSED, DELETED };

// Kept as a string on the wire (Message::category) for simplicity, same as
// how Listing keeps price as a free-form string. These are just the
// suggested canonical values used by the UI dropdown.
enum class OpportunityType { JOB, INTERNSHIP, VOLUNTEER, RESEARCH, OTHER };

inline std::string opportunityTypeToString(OpportunityType t)
{
    switch (t) {
        case OpportunityType::JOB:        return "Job";
        case OpportunityType::INTERNSHIP: return "Internship";
        case OpportunityType::VOLUNTEER:  return "Volunteer";
        case OpportunityType::RESEARCH:   return "Research";
        default:                          return "Other";
    }
}

inline OpportunityType opportunityTypeFromString(const std::string& s)
{
    if (s == "Job")        return OpportunityType::JOB;
    if (s == "Internship") return OpportunityType::INTERNSHIP;
    if (s == "Volunteer")  return OpportunityType::VOLUNTEER;
    if (s == "Research")   return OpportunityType::RESEARCH;
    return OpportunityType::OTHER;
}

struct Opportunity {
    std::string opportunityId;
    std::string posterUserId;
    std::string posterUsername;
    std::string title;          // role / opportunity name
    std::string description;    // stored in text field on the wire
    std::string category;       // "Job" / "Internship" / "Volunteer" / "Research" / "Other"
    std::string location;       // e.g. "Remote", "New Cairo Campus"
    std::string mediaUrl;       // optional flyer / listing link
    OpportunityStatus status = OpportunityStatus::ACTIVE;
    std::string createdAt;
};