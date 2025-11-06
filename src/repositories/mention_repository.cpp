#include "repositories/mention_repository.h"
#include <iostream>

namespace sohbet {
namespace repositories {

MentionRepository::MentionRepository(std::shared_ptr<db::Database> database)
    : database_(database) {}

bool MentionRepository::createMention(int post_id, int user_id) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = R"(
        INSERT OR IGNORE INTO post_mentions (post_id, user_id)
        VALUES (?, ?)
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, post_id);
    stmt.bindInt(2, user_id);

    return stmt.step() == SQLITE_DONE;
}

bool MentionRepository::deleteMention(int post_id, int user_id) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = R"(
        DELETE FROM post_mentions
        WHERE post_id = ? AND user_id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, post_id);
    stmt.bindInt(2, user_id);

    return stmt.step() == SQLITE_DONE;
}

std::vector<int> MentionRepository::findUserIdsByPostId(int post_id) {
    std::vector<int> user_ids;
    if (!database_ || !database_->isOpen()) return user_ids;

    const std::string sql = R"(
        SELECT user_id
        FROM post_mentions
        WHERE post_id = ?
        ORDER BY created_at
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return user_ids;

    stmt.bindInt(1, post_id);

    while (stmt.step() == SQLITE_ROW) {
        user_ids.push_back(stmt.getInt(0));
    }

    return user_ids;
}

std::vector<int> MentionRepository::findPostIdsByUserId(int user_id, int limit, int offset) {
    std::vector<int> post_ids;
    if (!database_ || !database_->isOpen()) return post_ids;

    const std::string sql = R"(
        SELECT post_id
        FROM post_mentions
        WHERE user_id = ?
        ORDER BY created_at DESC
        LIMIT ? OFFSET ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return post_ids;

    stmt.bindInt(1, user_id);
    stmt.bindInt(2, limit);
    stmt.bindInt(3, offset);

    while (stmt.step() == SQLITE_ROW) {
        post_ids.push_back(stmt.getInt(0));
    }

    return post_ids;
}

bool MentionRepository::createMentions(int post_id, const std::set<int>& user_ids) {
    bool all_success = true;

    for (int user_id : user_ids) {
        if (!createMention(post_id, user_id)) {
            all_success = false;
        }
    }

    return all_success;
}

bool MentionRepository::deleteMentionsByPostId(int post_id) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = "DELETE FROM post_mentions WHERE post_id = ?";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, post_id);
    return stmt.step() == SQLITE_DONE;
}

int MentionRepository::countMentionsForUser(int user_id) {
    if (!database_ || !database_->isOpen()) return 0;

    const std::string sql = R"(
        SELECT COUNT(*)
        FROM post_mentions
        WHERE user_id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return 0;

    stmt.bindInt(1, user_id);

    if (stmt.step() == SQLITE_ROW) {
        return stmt.getInt(0);
    }

    return 0;
}

} // namespace repositories
} // namespace sohbet
