#pragma once

#include "models/comment.h"
#include "db/database.h"
#include <memory>
#include <optional>
#include <vector>

namespace sohbet {
namespace repositories {

class CommentRepository {
public:
    explicit CommentRepository(std::shared_ptr<db::Database> database);

    // CRUD operations
    std::optional<Comment> create(Comment& comment);
    std::optional<Comment> findById(int id);
    std::vector<Comment> findByPostId(int post_id, int limit = 100, int offset = 0);
    std::vector<Comment> findReplies(int parent_comment_id, int limit = 50, int offset = 0);
    bool update(const Comment& comment);
    bool deleteById(int id);
    
    // Helper methods
    int getCommentCount(int post_id);

private:
    std::shared_ptr<db::Database> database_;
};

} // namespace repositories
} // namespace sohbet
