#include "services/FileStorageService.h"
#include "store/InMemoryStore.h"
#include "models/FileRecord.h"
#include "Session.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

namespace fs = std::filesystem;

//takes the database and a folder where the uploaded files will be saved
FileStorageService::FileStorageService(InMemoryStore& store, const std::string& uploadDir) : store_(store), uploadDir_(uploadDir)
{
    fs::create_directories(uploadDir_);
}

//upload a file
//The client sends:
//   msg.filename  = original filename  (e.g. "lecture_notes.pdf")
//   msg.text      = base64-encoded file content
//   msg.mediaUrl  = optional description
//
//server saves to disk under uploads/ and records the URL.
void FileStorageService::handleUpload(const Message& msg, std::shared_ptr<Session> sender)
{
    if (msg.filename.empty() || msg.text.empty())
    { sendError("filename and file content (text) are required", sender); return; }

    std::string fileId   = generateId();                //generate a unique ID name
    std::string saveName = fileId + "_" + msg.filename;
    std::string savePath = uploadDir_ + saveName;

    //open file and write the recieved data and close the file
    std::ofstream out(savePath, std::ios::binary);
    if (!out.is_open())
    { sendError("Server could not save file", sender); return; }
    out << msg.text;
    out.close();

    //build the record
    FileRecord record;
    record.fileId = fileId;
    record.uploaderUserId  = sender->userId();
    record.uploaderUsername= msg.sender.username;
    record.filename = msg.filename;
    record.url = "files/" + saveName;  // relative URL served by server
    record.fileSize = std::to_string(msg.text.size()) + " bytes";
    record.flagged = false;
    record.uploadedAt = currentTimestamp();

    if (!store_.addFile(record))
    { sendError("Failed to register file", sender); return; }

    std::cout << "File uploaded: " << record.filename << " by " << record.uploaderUsername << "\n";

    Message resp;
    resp.type = MessageType::MATERIAL_UPLOAD;
    resp.parentId = record.fileId;
    resp.filename = record.filename;
    resp.mediaUrl = record.url;
    resp.text = "File uploaded successfully";
    sender->send(resp);
}

//report file
void FileStorageService::handleReport(const Message& msg, std::shared_ptr<Session> sender)
{
    if (msg.parentId.empty())
    { sendError("parentId (fileId) is required", sender); return; }

    std::string reason = msg.text.empty() ? "No reason provided" : msg.text;

    if (!store_.flagFile(msg.parentId, reason))     //flag as reported
    { sendError("File not found", sender); return; }

    std::cout << "File flagged: " << msg.parentId
              << " reason: " << reason << "\n";

    sendOk("File reported. It will be reviewed by an admin.", sender);
}

//list all non-flagged files
void FileStorageService::handleList(const Message& msg, std::shared_ptr<Session> sender)
{
    auto files = store_.getAllFiles();

    for (auto& f : files)
    {
        Message resp;
        resp.type = MessageType::MATERIAL_UPLOAD;
        resp.parentId = f.fileId;
        resp.filename = f.filename;
        resp.mediaUrl = f.url;
        resp.text = f.fileSize;
        resp.sender.userId = f.uploaderUserId;
        resp.sender.username = f.uploaderUsername;
        resp.timestamp = f.uploadedAt;
        sender->send(resp);
    }

    if (files.empty())
        sendOk("No files available", sender);
}

//get one file record
void FileStorageService::handleGetFile(const Message& msg, std::shared_ptr<Session> sender)
{
    if (msg.parentId.empty())
    { sendError("parentId (fileId) is required", sender); return; }

    auto fileOpt = store_.findFile(msg.parentId);
    if (!fileOpt || fileOpt->flagged)
    { sendError("File not found or has been removed", sender); return; }

    Message resp;
    resp.type = MessageType::MATERIAL_UPLOAD;
    resp.parentId = fileOpt->fileId;
    resp.filename = fileOpt->filename;
    resp.mediaUrl = fileOpt->url;
    resp.text = fileOpt->fileSize;
    resp.sender.userId = fileOpt->uploaderUserId;
    resp.sender.username = fileOpt->uploaderUsername;
    resp.timestamp = fileOpt->uploadedAt;
    sender->send(resp);
}

std::string FileStorageService::generateId()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    std::ostringstream oss;
    oss << std::hex << dis(gen);
    return oss.str();
}

std::string FileStorageService::currentTimestamp()
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&t), "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

void FileStorageService::sendError(const std::string& reason, std::shared_ptr<Session> sender)
{
    Message m; m.type = MessageType::ERROR; m.text = reason;
    sender->send(m);
}

void FileStorageService::sendOk(const std::string& text, std::shared_ptr<Session> sender)
{
    Message m; m.type = MessageType::MATERIAL_REPORT; m.text = text;
    sender->send(m);
}