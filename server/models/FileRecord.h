#pragma once
#include <string>

struct FileRecord {
    std::string fileId;
    std::string uploaderUserId;
    std::string uploaderUsername;
    std::string filename;
    std::string url;           // served from server/uploads/
    std::string fileSize;
    bool        flagged = false;
    std::string flagReason;
    std::string uploadedAt;
};