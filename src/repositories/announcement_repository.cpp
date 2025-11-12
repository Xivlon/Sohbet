#include "repositories/announcement_repository.h"
#include <iostream>

namespace sohbet {
namespace repositories {

AnnouncementRepository::AnnouncementRepository(std::shared_ptr<db::Database> database)
    : database_(database) {}

std::optional<Announcement> AnnouncementRepository::create(Announcement& announcement) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        INSERT INTO group_announcements (group_id, author_id, title, content, is_pinned)
        VALUES (?, ?, ?, ?, ?)
        RETURNING id
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindInt(1, announcement.getGroupId());
    stmt.bindInt(2, announcement.getAuthorId());
    stmt.bindText(3, announcement.getTitle());
    stmt.bindText(4, announcement.getContent());
    stmt.bindInt(5, announcement.isPinned() ? 1 : 0);

    int result = stmt.step();
    if (result == SQLITE_ROW) {
        announcement.setId(stmt.getInt(0));
        // Call step() again to commit the transaction
        stmt.step();
        return announcement;
    }

    return std::nullopt;
}

std::optional<Announcement> AnnouncementRepository::findById(int id) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        SELECT a.id, a.group_id, a.author_id, a.title, a.content, a.is_pinned,
               a.created_at, a.updated_at, u.username, u.name
        FROM group_announcements a
        LEFT JOIN users u ON a.author_id = u.id
        WHERE a.id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindInt(1, id);

    if (stmt.step() == SQLITE_ROW) {
        Announcement announcement;
        announcement.setId(stmt.getInt(0));
        announcement.setGroupId(stmt.getInt(1));
        announcement.setAuthorId(stmt.getInt(2));
        announcement.setTitle(stmt.getText(3));
        announcement.setContent(stmt.getText(4));
        announcement.setPinned(stmt.getInt(5) != 0);
        announcement.setCreatedAt(stmt.getText(6));
        announcement.setUpdatedAt(stmt.getText(7));
        if (!stmt.isNull(8)) {
            announcement.setAuthorUsername(stmt.getText(8));
        }
        if (!stmt.isNull(9)) {
            announcement.setAuthorName(stmt.getText(9));
        }
        return announcement;
    }

    return std::nullopt;
}

std::vector<Announcement> AnnouncementRepository::findByGroupId(int group_id, bool pinned_only, int limit, int offset) {
    std::vector<Announcement> announcements;
    if (!database_ || !database_->isOpen()) return announcements;

    std::string sql = R"(
        SELECT a.id, a.group_id, a.author_id, a.title, a.content, a.is_pinned,
               a.created_at, a.updated_at, u.username, u.name
        FROM group_announcements a
        LEFT JOIN users u ON a.author_id = u.id
        WHERE a.group_id = ?
    )";

    if (pinned_only) {
        sql += " AND a.is_pinned = 1";
    }

    sql += " ORDER BY a.is_pinned DESC, a.created_at DESC LIMIT ? OFFSET ?";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return announcements;

    stmt.bindInt(1, group_id);
    stmt.bindInt(2, limit);
    stmt.bindInt(3, offset);

    while (stmt.step() == SQLITE_ROW) {
        Announcement announcement;
        announcement.setId(stmt.getInt(0));
        announcement.setGroupId(stmt.getInt(1));
        announcement.setAuthorId(stmt.getInt(2));
        announcement.setTitle(stmt.getText(3));
        announcement.setContent(stmt.getText(4));
        announcement.setPinned(stmt.getInt(5) != 0);
        announcement.setCreatedAt(stmt.getText(6));
        announcement.setUpdatedAt(stmt.getText(7));
        if (!stmt.isNull(8)) {
            announcement.setAuthorUsername(stmt.getText(8));
        }
        if (!stmt.isNull(9)) {
            announcement.setAuthorName(stmt.getText(9));
        }
        announcements.push_back(announcement);
    }

    return announcements;
}

bool AnnouncementRepository::update(const Announcement& announcement) {
    if (!database_ || !database_->isOpen() || !announcement.getId().has_value()) return false;

    const std::string sql = R"(
        UPDATE group_announcements
        SET title = ?, content = ?, is_pinned = ?, updated_at = CURRENT_TIMESTAMP
        WHERE id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindText(1, announcement.getTitle());
    stmt.bindText(2, announcement.getContent());
    stmt.bindInt(3, announcement.isPinned() ? 1 : 0);
    stmt.bindInt(4, announcement.getId().value());

    return stmt.step() == SQLITE_DONE;
}

bool AnnouncementRepository::deleteById(int id) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = "DELETE FROM group_announcements WHERE id = ?";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, id);
    return stmt.step() == SQLITE_DONE;
}

bool AnnouncementRepository::pin(int id) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = R"(
        UPDATE group_announcements
        SET is_pinned = 1, updated_at = CURRENT_TIMESTAMP
        WHERE id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, id);
    return stmt.step() == SQLITE_DONE;
}

bool AnnouncementRepository::unpin(int id) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = R"(
        UPDATE group_announcements
        SET is_pinned = 0, updated_at = CURRENT_TIMESTAMP
        WHERE id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, id);
    return stmt.step() == SQLITE_DONE;
}

std::vector<Announcement> AnnouncementRepository::findPinnedByGroupId(int group_id) {
    return findByGroupId(group_id, true, 50, 0);
}

bool AnnouncementRepository::canUserManage(int announcement_id, int user_id) {
    if (!database_ || !database_->isOpen()) return false;

    // Check if user is the author or a group admin/moderator
    const std::string sql = R"(
        SELECT 1
        FROM group_announcements a
        LEFT JOIN group_members gm ON a.group_id = gm.group_id AND gm.user_id = ?
        WHERE a.id = ?
        AND (a.author_id = ? OR gm.role IN ('admin', 'moderator'))
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, user_id);
    stmt.bindInt(2, announcement_id);
    stmt.bindInt(3, user_id);

    return stmt.step() == SQLITE_ROW;
}

} // namespace repositories
} // namespace sohbet
