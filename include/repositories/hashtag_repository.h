#pragma once

#include "models/hashtag.h"
#include "db/database.h"
#include <memory>
#include <optional>
#include <vector>
#include <set>

namespace sohbet {
namespace repositories {

class HashtagRepository {
public:
    explicit HashtagRepository(std::shared_ptr<db::Database> database);

    // CRUD operations
    std::optional<Hashtag> create(Hashtag& hashtag);
    std::optional<Hashtag> findById(int id);
    std::optional<Hashtag> findByTag(const std::string& tag);
    std::vector<Hashtag> findTrending(int limit = 10);
    std::vector<Hashtag> searchTags(const std::string& query, int limit = 20);
    bool update(const Hashtag& hashtag);
    bool deleteById(int id);

    // Post associations
    bool linkToPost(int hashtag_id, int post_id);
    bool unlinkFromPost(int hashtag_id, int post_id);
    std::vector<Hashtag> findByPostId(int post_id);

    // Bulk operations
    std::vector<Hashtag> findOrCreateTags(const std::set<std::string>& tags);
    bool linkTagsToPost(const std::vector<int>& hashtag_ids, int post_id);

    // Usage tracking
    bool incrementUsage(int hashtag_id);
    bool updateLastUsed(int hashtag_id);

private:
    std::shared_ptr<db::Database> database_;
};

} // namespace repositories
} // namespace sohbet
