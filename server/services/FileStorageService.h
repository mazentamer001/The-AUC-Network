#pragma once
#include <memory>
#include "Message.h"

class Session;
class InMemoryStore;

class FileStorageService {
public:
    explicit FileStorageService(InMemoryStore& store,
                                const std::string& uploadDir = "uploads/");

    void handleUpload  (const Message& msg, std::shared_ptr<Session> sender); // register file URL
    void handleReport  (const Message& msg, std::shared_ptr<Session> sender); // flag a file
    void handleList    (const Message& msg, std::shared_ptr<Session> sender); // list all files
    void handleGetFile (const Message& msg, std::shared_ptr<Session> sender); // get one file record

private:
    std::string generateId();
    std::string currentTimestamp();
    void sendError(const std::string& reason, std::shared_ptr<Session> sender);
    void sendOk   (const std::string& text,   std::shared_ptr<Session> sender);

    InMemoryStore& store_;
    std::string    uploadDir_;
};