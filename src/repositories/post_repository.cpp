#include "repositories/post_repository.h"
#include <iostream>

namespace sohbet {
namespace repositories {

PostRepository::PostRepository(std::shared_ptr<db::Database> database)
    : database_(database) {}

std::optional<Post> PostRepository::create(Post& post) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        INSERT INTO posts (author_id, author_type, content, media_urls, visibility, group_id)
        VALUES (?, ?, ?, ?, ?, ?)
        RETURNING id
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindInt(1, post.getAuthorId());
    stmt.bindText(2, post.getAuthorType());
    stmt.bindText(3, post.getContent());

    if (post.getMediaUrls().has_value()) {
        stmt.bindText(4, post.getMediaUrls().value());
    } else {
        stmt.bindNull(4);
    }

    stmt.bindText(5, post.getVisibility());

    if (post.getGroupId().has_value()) {
        stmt.bindInt(6, post.getGroupId().value());
    } else {
        stmt.bindNull(6);
    }

    int result = stmt.step();
    if (result == SQLITE_ROW) {
        post.setId(stmt.getInt(0));
        // Call step() again to commit the transaction
        stmt.step();
        return post;
    }

    return std::nullopt;
}

std::optional<Post> PostRepository::findById(int id) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        SELECT id, author_id, author_type, content, media_urls, visibility, 
               group_id, created_at, updated_at
        FROM posts WHERE id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindInt(1, id);

    if (stmt.step() == SQLITE_ROW) {
        Post post;
        post.setId(stmt.getInt(0));
        post.setAuthorId(stmt.getInt(1));
        post.setAuthorType(stmt.getText(2));
        post.setContent(stmt.getText(3));
        if (!stmt.isNull(4)) {
            post.setMediaUrls(stmt.getText(4));
        }
        post.setVisibility(stmt.getText(5));
        if (!stmt.isNull(6)) {
            post.setGroupId(stmt.getInt(6));
        }
        post.setCreatedAt(stmt.getText(7));
        post.setUpdatedAt(stmt.getText(8));
        return post;
    }

    return std::nullopt;
}

std::vector<Post> PostRepository::findByAuthor(int author_id, int limit, int offset) {
    std::vector<Post> posts;
    if (!database_ || !database_->isOpen()) return posts;

    const std::string sql = R"(
        SELECT p.id, p.author_id, p.author_type, p.content, p.media_urls, p.visibility,
               p.group_id, p.created_at, p.updated_at,
               u.username, u.name, u.avatar_url
        FROM posts p
        LEFT JOIN users u ON p.author_id = u.id
        WHERE p.author_id = ?
        ORDER BY p.created_at DESC
        LIMIT ? OFFSET ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return posts;

    stmt.bindInt(1, author_id);
    stmt.bindInt(2, limit);
    stmt.bindInt(3, offset);

    while (stmt.step() == SQLITE_ROW) {
        Post post;
        post.setId(stmt.getInt(0));
        post.setAuthorId(stmt.getInt(1));
        post.setAuthorType(stmt.getText(2));
        post.setContent(stmt.getText(3));
        if (!stmt.isNull(4)) {
            post.setMediaUrls(stmt.getText(4));
        }
        post.setVisibility(stmt.getText(5));
        if (!stmt.isNull(6)) {
            post.setGroupId(stmt.getInt(6));
        }
        post.setCreatedAt(stmt.getText(7));
        post.setUpdatedAt(stmt.getText(8));
        // Author information from JOIN
        if (!stmt.isNull(9)) {
            post.setAuthorUsername(stmt.getText(9));
        }
        if (!stmt.isNull(10)) {
            post.setAuthorName(stmt.getText(10));
        }
        if (!stmt.isNull(11)) {
            post.setAuthorAvatarUrl(stmt.getText(11));
        }
        posts.push_back(post);
    }

    return posts;
}

std::vector<Post> PostRepository::findFeedForUser(int user_id, int limit, int offset) {
    std::vector<Post> posts;
    if (!database_ || !database_->isOpen()) return posts;

    // Get posts from friends and user's own posts with author information
    const std::string sql = R"(
        SELECT DISTINCT p.id, p.author_id, p.author_type, p.content, p.media_urls,
               p.visibility, p.group_id, p.created_at, p.updated_at,
               u.username, u.name, u.avatar_url
        FROM posts p
        LEFT JOIN users u ON p.author_id = u.id
        LEFT JOIN friendships f ON (
            (f.requester_id = p.author_id AND f.addressee_id = ?) OR
            (f.addressee_id = p.author_id AND f.requester_id = ?)
        )
        WHERE (
            p.visibility = 'public' OR
            p.author_id = ? OR
            (p.visibility = 'friends' AND f.status = 'accepted')
        )
        ORDER BY p.created_at DESC
        LIMIT ? OFFSET ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return posts;

    stmt.bindInt(1, user_id);
    stmt.bindInt(2, user_id);
    stmt.bindInt(3, user_id);
    stmt.bindInt(4, limit);
    stmt.bindInt(5, offset);

    while (stmt.step() == SQLITE_ROW) {
        Post post;
        post.setId(stmt.getInt(0));
        post.setAuthorId(stmt.getInt(1));
        post.setAuthorType(stmt.getText(2));
        post.setContent(stmt.getText(3));
        if (!stmt.isNull(4)) {
            post.setMediaUrls(stmt.getText(4));
        }
        post.setVisibility(stmt.getText(5));
        if (!stmt.isNull(6)) {
            post.setGroupId(stmt.getInt(6));
        }
        post.setCreatedAt(stmt.getText(7));
        post.setUpdatedAt(stmt.getText(8));
        // Author information from JOIN
        if (!stmt.isNull(9)) {
            post.setAuthorUsername(stmt.getText(9));
        }
        if (!stmt.isNull(10)) {
            post.setAuthorName(stmt.getText(10));
        }
        if (!stmt.isNull(11)) {
            post.setAuthorAvatarUrl(stmt.getText(11));
        }
        posts.push_back(post);
    }

    return posts;
}

std::vector<Post> PostRepository::findByGroupId(int group_id, int limit, int offset) {
    std::vector<Post> posts;
    if (!database_ || !database_->isOpen()) return posts;

    const std::string sql = R"(
        SELECT p.id, p.author_id, p.author_type, p.content, p.media_urls, p.visibility,
               p.group_id, p.created_at, p.updated_at,
               u.username, u.name, u.avatar_url
        FROM posts p
        LEFT JOIN users u ON p.author_id = u.id
        WHERE p.group_id = ?
        ORDER BY p.created_at DESC
        LIMIT ? OFFSET ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return posts;

    stmt.bindInt(1, group_id);
    stmt.bindInt(2, limit);
    stmt.bindInt(3, offset);

    while (stmt.step() == SQLITE_ROW) {
        Post post;
        post.setId(stmt.getInt(0));
        post.setAuthorId(stmt.getInt(1));
        post.setAuthorType(stmt.getText(2));
        post.setContent(stmt.getText(3));
        if (!stmt.isNull(4)) {
            post.setMediaUrls(stmt.getText(4));
        }
        post.setVisibility(stmt.getText(5));
        if (!stmt.isNull(6)) {
            post.setGroupId(stmt.getInt(6));
        }
        post.setCreatedAt(stmt.getText(7));
        post.setUpdatedAt(stmt.getText(8));
        // Author information from JOIN
        if (!stmt.isNull(9)) {
            post.setAuthorUsername(stmt.getText(9));
        }
        if (!stmt.isNull(10)) {
            post.setAuthorName(stmt.getText(10));
        }
        if (!stmt.isNull(11)) {
            post.setAuthorAvatarUrl(stmt.getText(11));
        }
        posts.push_back(post);
    }

    return posts;
}

bool PostRepository::update(const Post& post) {
    if (!database_ || !database_->isOpen()) return false;
    if (!post.getId().has_value()) return false;

    const std::string sql = R"(
        UPDATE posts 
        SET content = ?, media_urls = ?, visibility = ?, updated_at = CURRENT_TIMESTAMP
        WHERE id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindText(1, post.getContent());
    
    if (post.getMediaUrls().has_value()) {
        stmt.bindText(2, post.getMediaUrls().value());
    } else {
        stmt.bindNull(2);
    }
    
    stmt.bindText(3, post.getVisibility());
    stmt.bindInt(4, post.getId().value());

    return stmt.step() == SQLITE_DONE;
}

bool PostRepository::deleteById(int id) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = "DELETE FROM posts WHERE id = ?";
    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, id);
    return stmt.step() == SQLITE_DONE;
}

bool PostRepository::canUserViewPost(int post_id, int viewer_id) {
    auto post = findById(post_id);
    if (!post.has_value()) return false;

    // Public posts can be viewed by anyone
    if (post->getVisibility() == Post::VISIBILITY_PUBLIC) return true;

    // Author can always view their own posts
    if (post->getAuthorId() == viewer_id) return true;

    // Friends visibility - check if users are friends
    if (post->getVisibility() == Post::VISIBILITY_FRIENDS) {
        return areFriends(post->getAuthorId(), viewer_id);
    }

    // Group visibility - check if viewer is member of the group
    if (post->getVisibility() == Post::VISIBILITY_GROUP && post->getGroupId().has_value()) {
        // TODO: Implement group membership check
        return false;
    }

    // Private posts can only be viewed by the author
    return false;
}

bool PostRepository::areFriends(int user1_id, int user2_id) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = R"(
        SELECT COUNT(*) FROM friendships 
        WHERE ((requester_id = ? AND addressee_id = ?) 
            OR (requester_id = ? AND addressee_id = ?))
          AND status = 'accepted'
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, user1_id);
    stmt.bindInt(2, user2_id);
    stmt.bindInt(3, user2_id);
    stmt.bindInt(4, user1_id);

    if (stmt.step() == SQLITE_ROW) {
        return stmt.getInt(0) > 0;
    }

    return false;
}

bool PostRepository::addReaction(int post_id, int user_id, const std::string& reaction_type) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = R"(
        INSERT OR REPLACE INTO post_reactions (post_id, user_id, reaction_type)
        VALUES (?, ?, ?)
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, post_id);
    stmt.bindInt(2, user_id);
    stmt.bindText(3, reaction_type);

    return stmt.step() == SQLITE_DONE;
}

bool PostRepository::removeReaction(int post_id, int user_id, const std::string& reaction_type) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = R"(
        DELETE FROM post_reactions 
        WHERE post_id = ? AND user_id = ? AND reaction_type = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, post_id);
    stmt.bindInt(2, user_id);
    stmt.bindText(3, reaction_type);

    return stmt.step() == SQLITE_DONE;
}

int PostRepository::getReactionCount(int post_id, const std::string& reaction_type) {
    if (!database_ || !database_->isOpen()) return 0;

    std::string sql = "SELECT COUNT(*) FROM post_reactions WHERE post_id = ?";
    
    if (!reaction_type.empty()) {
        sql += " AND reaction_type = ?";
    }

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return 0;

    stmt.bindInt(1, post_id);
    if (!reaction_type.empty()) {
        stmt.bindText(2, reaction_type);
    }

    if (stmt.step() == SQLITE_ROW) {
        return stmt.getInt(0);
    }

    return 0;
}

} // namespace repositories
} // namespace sohbet
