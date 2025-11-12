#include "repositories/comment_repository.h"
#include <iostream>

namespace sohbet {
namespace repositories {

CommentRepository::CommentRepository(std::shared_ptr<db::Database> database)
    : database_(database) {}

std::optional<Comment> CommentRepository::create(Comment& comment) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        INSERT INTO comments (post_id, parent_id, author_id, content)
        VALUES (?, ?, ?, ?)
        RETURNING id
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindInt(1, comment.getPostId());

    if (comment.getParentId().has_value()) {
        stmt.bindInt(2, comment.getParentId().value());
    } else {
        stmt.bindNull(2);
    }

    stmt.bindInt(3, comment.getAuthorId());
    stmt.bindText(4, comment.getContent());

    int result = stmt.step();
    if (result == SQLITE_ROW) {
        comment.setId(stmt.getInt(0));
        // Call step() again to commit the transaction
        stmt.step();
        return comment;
    }

    return std::nullopt;
}

std::optional<Comment> CommentRepository::findById(int id) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        SELECT id, post_id, parent_id, author_id, content, created_at, updated_at
        FROM comments WHERE id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindInt(1, id);

    if (stmt.step() == SQLITE_ROW) {
        Comment comment;
        comment.setId(stmt.getInt(0));
        comment.setPostId(stmt.getInt(1));
        if (!stmt.isNull(2)) {
            comment.setParentId(stmt.getInt(2));
        }
        comment.setAuthorId(stmt.getInt(3));
        comment.setContent(stmt.getText(4));
        comment.setCreatedAt(stmt.getText(5));
        comment.setUpdatedAt(stmt.getText(6));
        return comment;
    }

    return std::nullopt;
}

std::vector<Comment> CommentRepository::findByPostId(int post_id, int limit, int offset) {
    std::vector<Comment> comments;
    if (!database_ || !database_->isOpen()) return comments;

    // Get top-level comments first, ordered by creation time
    const std::string sql = R"(
        SELECT id, post_id, parent_id, author_id, content, created_at, updated_at
        FROM comments 
        WHERE post_id = ? AND parent_id IS NULL
        ORDER BY created_at ASC
        LIMIT ? OFFSET ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return comments;

    stmt.bindInt(1, post_id);
    stmt.bindInt(2, limit);
    stmt.bindInt(3, offset);

    while (stmt.step() == SQLITE_ROW) {
        Comment comment;
        comment.setId(stmt.getInt(0));
        comment.setPostId(stmt.getInt(1));
        if (!stmt.isNull(2)) {
            comment.setParentId(stmt.getInt(2));
        }
        comment.setAuthorId(stmt.getInt(3));
        comment.setContent(stmt.getText(4));
        comment.setCreatedAt(stmt.getText(5));
        comment.setUpdatedAt(stmt.getText(6));
        comments.push_back(comment);
    }

    return comments;
}

std::vector<Comment> CommentRepository::findReplies(int parent_comment_id, int limit, int offset) {
    std::vector<Comment> comments;
    if (!database_ || !database_->isOpen()) return comments;

    const std::string sql = R"(
        SELECT id, post_id, parent_id, author_id, content, created_at, updated_at
        FROM comments 
        WHERE parent_id = ?
        ORDER BY created_at ASC
        LIMIT ? OFFSET ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return comments;

    stmt.bindInt(1, parent_comment_id);
    stmt.bindInt(2, limit);
    stmt.bindInt(3, offset);

    while (stmt.step() == SQLITE_ROW) {
        Comment comment;
        comment.setId(stmt.getInt(0));
        comment.setPostId(stmt.getInt(1));
        if (!stmt.isNull(2)) {
            comment.setParentId(stmt.getInt(2));
        }
        comment.setAuthorId(stmt.getInt(3));
        comment.setContent(stmt.getText(4));
        comment.setCreatedAt(stmt.getText(5));
        comment.setUpdatedAt(stmt.getText(6));
        comments.push_back(comment);
    }

    return comments;
}

bool CommentRepository::update(const Comment& comment) {
    if (!database_ || !database_->isOpen()) return false;
    if (!comment.getId().has_value()) return false;

    const std::string sql = R"(
        UPDATE comments 
        SET content = ?, updated_at = CURRENT_TIMESTAMP
        WHERE id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindText(1, comment.getContent());
    stmt.bindInt(2, comment.getId().value());

    return stmt.step() == SQLITE_DONE;
}

bool CommentRepository::deleteById(int id) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = "DELETE FROM comments WHERE id = ?";
    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, id);
    return stmt.step() == SQLITE_DONE;
}

int CommentRepository::getCommentCount(int post_id) {
    if (!database_ || !database_->isOpen()) return 0;

    const std::string sql = "SELECT COUNT(*) FROM comments WHERE post_id = ?";
    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return 0;

    stmt.bindInt(1, post_id);

    if (stmt.step() == SQLITE_ROW) {
        return stmt.getInt(0);
    }

    return 0;
}

} // namespace repositories
} // namespace sohbet
