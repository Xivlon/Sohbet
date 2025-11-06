#include "repositories/hashtag_repository.h"
#include <iostream>
#include <algorithm>

namespace sohbet {
namespace repositories {

HashtagRepository::HashtagRepository(std::shared_ptr<db::Database> database)
    : database_(database) {}

std::optional<Hashtag> HashtagRepository::create(Hashtag& hashtag) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        INSERT INTO hashtags (tag, usage_count)
        VALUES (?, ?)
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindText(1, hashtag.getTag());
    stmt.bindInt(2, hashtag.getUsageCount());

    int result = stmt.step();
    if (result == SQLITE_DONE) {
        hashtag.setId(static_cast<int>(database_->lastInsertRowId()));
        return hashtag;
    }

    return std::nullopt;
}

std::optional<Hashtag> HashtagRepository::findById(int id) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        SELECT id, tag, usage_count, created_at, last_used_at
        FROM hashtags WHERE id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindInt(1, id);

    if (stmt.step() == SQLITE_ROW) {
        Hashtag hashtag;
        hashtag.setId(stmt.getInt(0));
        hashtag.setTag(stmt.getText(1));
        hashtag.setUsageCount(stmt.getInt(2));
        hashtag.setCreatedAt(stmt.getText(3));
        hashtag.setLastUsedAt(stmt.getText(4));
        return hashtag;
    }

    return std::nullopt;
}

std::optional<Hashtag> HashtagRepository::findByTag(const std::string& tag) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        SELECT id, tag, usage_count, created_at, last_used_at
        FROM hashtags WHERE tag = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindText(1, tag);

    if (stmt.step() == SQLITE_ROW) {
        Hashtag hashtag;
        hashtag.setId(stmt.getInt(0));
        hashtag.setTag(stmt.getText(1));
        hashtag.setUsageCount(stmt.getInt(2));
        hashtag.setCreatedAt(stmt.getText(3));
        hashtag.setLastUsedAt(stmt.getText(4));
        return hashtag;
    }

    return std::nullopt;
}

std::vector<Hashtag> HashtagRepository::findTrending(int limit) {
    std::vector<Hashtag> hashtags;
    if (!database_ || !database_->isOpen()) return hashtags;

    const std::string sql = R"(
        SELECT id, tag, usage_count, created_at, last_used_at
        FROM hashtags
        ORDER BY usage_count DESC, last_used_at DESC
        LIMIT ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return hashtags;

    stmt.bindInt(1, limit);

    while (stmt.step() == SQLITE_ROW) {
        Hashtag hashtag;
        hashtag.setId(stmt.getInt(0));
        hashtag.setTag(stmt.getText(1));
        hashtag.setUsageCount(stmt.getInt(2));
        hashtag.setCreatedAt(stmt.getText(3));
        hashtag.setLastUsedAt(stmt.getText(4));
        hashtags.push_back(hashtag);
    }

    return hashtags;
}

std::vector<Hashtag> HashtagRepository::searchTags(const std::string& query, int limit) {
    std::vector<Hashtag> hashtags;
    if (!database_ || !database_->isOpen()) return hashtags;

    const std::string sql = R"(
        SELECT id, tag, usage_count, created_at, last_used_at
        FROM hashtags
        WHERE tag LIKE ?
        ORDER BY usage_count DESC
        LIMIT ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return hashtags;

    std::string search_pattern = query + "%";
    stmt.bindText(1, search_pattern);
    stmt.bindInt(2, limit);

    while (stmt.step() == SQLITE_ROW) {
        Hashtag hashtag;
        hashtag.setId(stmt.getInt(0));
        hashtag.setTag(stmt.getText(1));
        hashtag.setUsageCount(stmt.getInt(2));
        hashtag.setCreatedAt(stmt.getText(3));
        hashtag.setLastUsedAt(stmt.getText(4));
        hashtags.push_back(hashtag);
    }

    return hashtags;
}

bool HashtagRepository::update(const Hashtag& hashtag) {
    if (!database_ || !database_->isOpen() || !hashtag.getId().has_value()) return false;

    const std::string sql = R"(
        UPDATE hashtags
        SET tag = ?, usage_count = ?, last_used_at = CURRENT_TIMESTAMP
        WHERE id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindText(1, hashtag.getTag());
    stmt.bindInt(2, hashtag.getUsageCount());
    stmt.bindInt(3, hashtag.getId().value());

    return stmt.step() == SQLITE_DONE;
}

bool HashtagRepository::deleteById(int id) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = "DELETE FROM hashtags WHERE id = ?";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, id);
    return stmt.step() == SQLITE_DONE;
}

bool HashtagRepository::linkToPost(int hashtag_id, int post_id) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = R"(
        INSERT OR IGNORE INTO post_hashtags (post_id, hashtag_id)
        VALUES (?, ?)
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, post_id);
    stmt.bindInt(2, hashtag_id);

    return stmt.step() == SQLITE_DONE;
}

bool HashtagRepository::unlinkFromPost(int hashtag_id, int post_id) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = R"(
        DELETE FROM post_hashtags
        WHERE hashtag_id = ? AND post_id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, hashtag_id);
    stmt.bindInt(2, post_id);

    return stmt.step() == SQLITE_DONE;
}

std::vector<Hashtag> HashtagRepository::findByPostId(int post_id) {
    std::vector<Hashtag> hashtags;
    if (!database_ || !database_->isOpen()) return hashtags;

    const std::string sql = R"(
        SELECT h.id, h.tag, h.usage_count, h.created_at, h.last_used_at
        FROM hashtags h
        INNER JOIN post_hashtags ph ON h.id = ph.hashtag_id
        WHERE ph.post_id = ?
        ORDER BY h.tag
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return hashtags;

    stmt.bindInt(1, post_id);

    while (stmt.step() == SQLITE_ROW) {
        Hashtag hashtag;
        hashtag.setId(stmt.getInt(0));
        hashtag.setTag(stmt.getText(1));
        hashtag.setUsageCount(stmt.getInt(2));
        hashtag.setCreatedAt(stmt.getText(3));
        hashtag.setLastUsedAt(stmt.getText(4));
        hashtags.push_back(hashtag);
    }

    return hashtags;
}

std::vector<Hashtag> HashtagRepository::findOrCreateTags(const std::set<std::string>& tags) {
    std::vector<Hashtag> result;

    for (const auto& tag : tags) {
        auto existing = findByTag(tag);
        if (existing.has_value()) {
            result.push_back(existing.value());
        } else {
            Hashtag new_hashtag(tag);
            auto created = create(new_hashtag);
            if (created.has_value()) {
                result.push_back(created.value());
            }
        }
    }

    return result;
}

bool HashtagRepository::linkTagsToPost(const std::vector<int>& hashtag_ids, int post_id) {
    bool all_success = true;

    for (int hashtag_id : hashtag_ids) {
        if (!linkToPost(hashtag_id, post_id)) {
            all_success = false;
        }
        // Update usage count and last used time
        incrementUsage(hashtag_id);
        updateLastUsed(hashtag_id);
    }

    return all_success;
}

bool HashtagRepository::incrementUsage(int hashtag_id) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = R"(
        UPDATE hashtags
        SET usage_count = usage_count + 1
        WHERE id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, hashtag_id);
    return stmt.step() == SQLITE_DONE;
}

bool HashtagRepository::updateLastUsed(int hashtag_id) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = R"(
        UPDATE hashtags
        SET last_used_at = CURRENT_TIMESTAMP
        WHERE id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, hashtag_id);
    return stmt.step() == SQLITE_DONE;
}

} // namespace repositories
} // namespace sohbet
