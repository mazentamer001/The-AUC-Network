#include "services/MarketplaceService.h"
#include "store/InMemoryStore.h"
#include "models/Listing.h"
#include "models/ChatRoom.h"
#include "Server.h"
#include "Session.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

MarketplaceService::MarketplaceService(InMemoryStore& store) : store_(store) {}

//post a listing
void MarketplaceService::handlePost(const Message& msg, std::shared_ptr<Session> sender)
{
    if (msg.title.empty() || msg.price.empty())
    { sendError("title and price are required", sender); return; }

    //create Listing
    Listing listing;
    listing.listingId = generateId();
    listing.sellerUserId = sender->userId();
    listing.sellerUsername = msg.sender.username;
    listing.title = msg.title;
    listing.description = msg.text;
    listing.price = msg.price;
    listing.mediaUrl = msg.mediaUrl;
    listing.status = ListingStatus::ACTIVE;
    listing.createdAt = currentTimestamp();

    if (!store_.addListing(listing))    //store it 
    { sendError("Failed to create listing", sender); return; }

    std::cout << "Listing created: " << listing.title << " by " << listing.sellerUsername << "\n";

    //create confirmation message to client
    Message resp;
    resp.type = MessageType::MARKET_POST;
    resp.parentId = listing.listingId;
    resp.title = listing.title;
    resp.price = listing.price;
    resp.text = listing.description;
    resp.mediaUrl = listing.mediaUrl;
    resp.sender.username = listing.sellerUsername;
    resp.sender.userId = listing.sellerUserId;
    sender->send(resp);

    if (server_) server_->broadcast(resp, sender);
}

//delete own listing
void MarketplaceService::handleDelete(const Message& msg, std::shared_ptr<Session> sender)
{
    if (msg.parentId.empty())
    { sendError("listingId (parentId) is required", sender); return; }

    if (!store_.deleteListing(msg.parentId, sender->userId()))
    { sendError("Listing not found or you are not the seller", sender); return; }

    Message resp;
    resp.type = MessageType::MARKET_DELETE;
    resp.parentId = msg.parentId;
    sender->send(resp);
    if (server_) server_->broadcast(resp, sender);
}

//search listings
void MarketplaceService::handleSearch(const Message& msg, std::shared_ptr<Session> sender)
{
    // msg.text is the search query — empty = return all active listings
    auto listings = store_.searchListings(msg.text);

    for (auto& l : listings)
    {
        Message resp;
        resp.type = MessageType::MARKET_POST;
        resp.parentId = l.listingId;
        resp.title = l.title;
        resp.text = l.description;
        resp.price = l.price;
        resp.mediaUrl = l.mediaUrl;
        resp.sender.userId = l.sellerUserId;
        resp.sender.username = l.sellerUsername;
        resp.timestamp = l.createdAt;
        sender->send(resp);
    }

    if (listings.empty())
        sendOk("No listings found", sender);
}

//inquiry / buy
//Opens a 1-on-1 direct chat room between buyer and seller
//then sends the seller a notification with the listing details
void MarketplaceService::handleInquiry(const Message& msg, std::shared_ptr<Session> sender)
{
    if (msg.parentId.empty())
    { sendError("listingId (parentId) is required", sender); return; }

    auto listingOpt = store_.findListing(msg.parentId);
    if (!listingOpt)
    { sendError("Listing not found", sender); return; }

    if (listingOpt->status != ListingStatus::ACTIVE)
    { sendError("This listing is no longer available", sender); return; }

    if (listingOpt->sellerUserId == sender->userId())
    { sendError("You cannot inquire on your own listing", sender); return; }

    //build deterministic direct room ID between buyer and seller (same in chatservice)
    std::string uid1 = sender->userId();
    std::string uid2 = listingOpt->sellerUserId;
    if (uid1 > uid2) std::swap(uid1, uid2);
    std::string directRoomId = "direct:" + uid1 + ":" + uid2;

    //create if not exists
    if (!store_.findRoom(directRoomId))
    {
        ChatRoom room;
        room.roomId = directRoomId;
        room.name = "Direct";
        room.type = RoomType::DIRECT;
        room.creatorId = sender->userId();
        room.memberIds = { uid1, uid2 };
        room.createdAt = currentTimestamp();
        store_.createRoom(room);
    }

    //notify seller via direct message
    Message notif;
    notif.type = MessageType::MARKET_INQUIRY;
    notif.roomId = directRoomId;
    notif.sender.userId = sender->userId();
    notif.sender.username = msg.sender.username;
    notif.title = listingOpt->title;
    notif.price = listingOpt->price;
    notif.text = msg.text.empty() ? msg.sender.username + " is interested in your listing: " + listingOpt->title : msg.text;
    notif.timestamp = currentTimestamp();

    if(server_) server_->sendTo(listingOpt->sellerUserId, notif);

    //confirm to buyer with the room ID so they can follow up
    Message confirm;
    confirm.type    = MessageType::MARKET_INQUIRY;
    confirm.text    = "Inquiry sent to seller. Use roomId to continue the chat.";
    confirm.roomId  = directRoomId;
    sender->send(confirm);
}

std::string MarketplaceService::generateId()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    std::ostringstream oss;
    oss << std::hex << dis(gen);
    return oss.str();
}

std::string MarketplaceService::currentTimestamp()
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&t), "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

void MarketplaceService::sendError(const std::string& reason, std::shared_ptr<Session> sender)
{
    Message m; m.type = MessageType::ERROR; m.text = reason;
    sender->send(m);
}

void MarketplaceService::sendOk(const std::string& text, std::shared_ptr<Session> sender)
{
    Message m; m.type = MessageType::MARKET_POST; m.text = text;
    sender->send(m);
}