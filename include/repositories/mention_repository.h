#pragma once

#include "db/database.h"
#include <memory>
#include <vector>
#include <set>

namespace sohbet {
namespace repositories {

class MentionRepository {
public:
    explicit MentionRepository(std::shared_ptr<db::Database> database);

    // CRUD operations
    bool createMention(int post_id, int user_id);
    bool deleteMention(int post_id, int user_id);
    std::vector<int> findUserIdsByPostId(int post_id);
    std::vector<int> findPostIdsByUserId(int user_id, int limit = 50, int offset = 0);

    // Bulk operations
    bool createMentions(int post_id, const std::set<int>& user_ids);
    bool deleteMentionsByPostId(int post_id);

    // Count operations
    int countMentionsForUser(int user_id);

private:
    std::shared_ptr<db::Database> database_;
};

} // namespace repositories
} // namespace sohbet
