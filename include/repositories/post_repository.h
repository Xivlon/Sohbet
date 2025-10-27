#pragma once

#include "models/post.h"
#include "db/database.h"
#include <memory>
#include <optional>
#include <vector>

namespace sohbet {
namespace repositories {

class PostRepository {
public:
    explicit PostRepository(std::shared_ptr<db::Database> database);

    // CRUD operations
    std::optional<Post> create(Post& post);
    std::optional<Post> findById(int id);
    std::vector<Post> findByAuthor(int author_id, int limit = 50, int offset = 0);
    std::vector<Post> findFeedForUser(int user_id, int limit = 50, int offset = 0);
    std::vector<Post> findByGroupId(int group_id, int limit = 50, int offset = 0);
    bool update(const Post& post);
    bool deleteById(int id);
    
    // Visibility check
    bool canUserViewPost(int post_id, int viewer_id);
    
    // Reactions
    bool addReaction(int post_id, int user_id, const std::string& reaction_type);
    bool removeReaction(int post_id, int user_id, const std::string& reaction_type);
    int getReactionCount(int post_id, const std::string& reaction_type = "");

private:
    std::shared_ptr<db::Database> database_;
    bool areFriends(int user1_id, int user2_id);
};

} // namespace repositories
} // namespace sohbet
