#pragma once
#include <string>

enum class ListingStatus { ACTIVE, SOLD, DELETED };

struct Listing {
    std::string   listingId;
    std::string   sellerUserId;
    std::string   sellerUsername;
    std::string   title;
    std::string   description;     // stored in text field
    std::string   price;
    std::string   mediaUrl;        // photo of item
    ListingStatus status = ListingStatus::ACTIVE;
    std::string   createdAt;
};