#pragma once

#include "models/media.h"
#include "db/database.h"
#include <memory>
#include <optional>
#include <vector>
#include <string>

namespace sohbet {
namespace repositories {

class MediaRepository {
public:
    explicit MediaRepository(std::shared_ptr<db::Database> database);

    // Create a new media record
    std::optional<Media> create(Media& media);
    
    // Find media by ID
    std::optional<Media> findById(int id);
    
    // Find user's media by type (e.g., "avatar", "banner")
    std::optional<Media> findByUserAndType(int user_id, const std::string& media_type);
    
    // Get all media for a user
    std::vector<Media> findByUser(int user_id);
    
    // Update media URL
    bool updateUrl(int id, const std::string& url);
    
    // Delete media
    bool deleteById(int id);
    
    // Delete user's media by type
    bool deleteByUserAndType(int user_id, const std::string& media_type);

private:
    std::shared_ptr<db::Database> database_;
    
    // Helper to convert DB row to Media object
    Media mediaFromStatement(db::Statement& stmt);
};

} // namespace repositories
} // namespace sohbet
