#include "repositories/group_repository.h"
#include <iostream>

namespace sohbet {
namespace repositories {

GroupRepository::GroupRepository(std::shared_ptr<db::Database> database)
    : database_(database) {}

std::optional<Group> GroupRepository::create(Group& group) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        INSERT INTO groups (name, description, creator_id, privacy)
        VALUES (?, ?, ?, ?)
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindText(1, group.getName());
    if (group.getDescription().has_value()) {
        stmt.bindText(2, group.getDescription().value());
    } else {
        stmt.bindNull(2);
    }
    stmt.bindInt(3, group.getCreatorId());
    stmt.bindText(4, group.getPrivacy());

    int result = stmt.step();
    if (result == SQLITE_DONE) {
        int group_id = static_cast<int>(database_->lastInsertRowId());
        group.setId(group_id);
        
        // Automatically add creator as admin member
        addMember(group_id, group.getCreatorId(), "admin");
        
        return group;
    }

    return std::nullopt;
}

std::optional<Group> GroupRepository::findById(int id) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        SELECT id, name, description, creator_id, privacy, created_at, updated_at
        FROM groups WHERE id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindInt(1, id);

    if (stmt.step() == SQLITE_ROW) {
        Group group;
        group.setId(stmt.getInt(0));
        group.setName(stmt.getText(1));
        if (!stmt.isNull(2)) {
            group.setDescription(stmt.getText(2));
        }
        group.setCreatorId(stmt.getInt(3));
        group.setPrivacy(stmt.getText(4));
        group.setCreatedAt(stmt.getText(5));
        group.setUpdatedAt(stmt.getText(6));
        return group;
    }

    return std::nullopt;
}

std::vector<Group> GroupRepository::findAll(int limit, int offset) {
    std::vector<Group> groups;
    if (!database_ || !database_->isOpen()) return groups;

    const std::string sql = R"(
        SELECT id, name, description, creator_id, privacy, created_at, updated_at
        FROM groups
        ORDER BY created_at DESC
        LIMIT ? OFFSET ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return groups;

    stmt.bindInt(1, limit);
    stmt.bindInt(2, offset);

    while (stmt.step() == SQLITE_ROW) {
        Group group;
        group.setId(stmt.getInt(0));
        group.setName(stmt.getText(1));
        if (!stmt.isNull(2)) {
            group.setDescription(stmt.getText(2));
        }
        group.setCreatorId(stmt.getInt(3));
        group.setPrivacy(stmt.getText(4));
        group.setCreatedAt(stmt.getText(5));
        group.setUpdatedAt(stmt.getText(6));
        groups.push_back(group);
    }

    return groups;
}

std::vector<Group> GroupRepository::findByCreator(int creator_id) {
    std::vector<Group> groups;
    if (!database_ || !database_->isOpen()) return groups;

    const std::string sql = R"(
        SELECT id, name, description, creator_id, privacy, created_at, updated_at
        FROM groups
        WHERE creator_id = ?
        ORDER BY created_at DESC
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return groups;

    stmt.bindInt(1, creator_id);

    while (stmt.step() == SQLITE_ROW) {
        Group group;
        group.setId(stmt.getInt(0));
        group.setName(stmt.getText(1));
        if (!stmt.isNull(2)) {
            group.setDescription(stmt.getText(2));
        }
        group.setCreatorId(stmt.getInt(3));
        group.setPrivacy(stmt.getText(4));
        group.setCreatedAt(stmt.getText(5));
        group.setUpdatedAt(stmt.getText(6));
        groups.push_back(group);
    }

    return groups;
}

std::vector<Group> GroupRepository::findByMember(int user_id) {
    std::vector<Group> groups;
    if (!database_ || !database_->isOpen()) return groups;

    const std::string sql = R"(
        SELECT g.id, g.name, g.description, g.creator_id, g.privacy, g.created_at, g.updated_at
        FROM groups g
        INNER JOIN group_members gm ON g.id = gm.group_id
        WHERE gm.user_id = ?
        ORDER BY g.created_at DESC
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return groups;

    stmt.bindInt(1, user_id);

    while (stmt.step() == SQLITE_ROW) {
        Group group;
        group.setId(stmt.getInt(0));
        group.setName(stmt.getText(1));
        if (!stmt.isNull(2)) {
            group.setDescription(stmt.getText(2));
        }
        group.setCreatorId(stmt.getInt(3));
        group.setPrivacy(stmt.getText(4));
        group.setCreatedAt(stmt.getText(5));
        group.setUpdatedAt(stmt.getText(6));
        groups.push_back(group);
    }

    return groups;
}

bool GroupRepository::update(const Group& group) {
    if (!database_ || !database_->isOpen()) return false;
    if (!group.getId().has_value()) return false;

    const std::string sql = R"(
        UPDATE groups 
        SET name = ?, description = ?, privacy = ?, updated_at = CURRENT_TIMESTAMP
        WHERE id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindText(1, group.getName());
    if (group.getDescription().has_value()) {
        stmt.bindText(2, group.getDescription().value());
    } else {
        stmt.bindNull(2);
    }
    stmt.bindText(3, group.getPrivacy());
    stmt.bindInt(4, group.getId().value());

    return stmt.step() == SQLITE_DONE;
}

bool GroupRepository::deleteById(int id) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = "DELETE FROM groups WHERE id = ?";
    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, id);
    return stmt.step() == SQLITE_DONE;
}

bool GroupRepository::addMember(int group_id, int user_id, const std::string& role) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = R"(
        INSERT OR IGNORE INTO group_members (group_id, user_id, role)
        VALUES (?, ?, ?)
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, group_id);
    stmt.bindInt(2, user_id);
    stmt.bindText(3, role);

    return stmt.step() == SQLITE_DONE;
}

bool GroupRepository::removeMember(int group_id, int user_id) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = R"(
        DELETE FROM group_members WHERE group_id = ? AND user_id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, group_id);
    stmt.bindInt(2, user_id);

    return stmt.step() == SQLITE_DONE;
}

bool GroupRepository::updateMemberRole(int group_id, int user_id, const std::string& role) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = R"(
        UPDATE group_members SET role = ? WHERE group_id = ? AND user_id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindText(1, role);
    stmt.bindInt(2, group_id);
    stmt.bindInt(3, user_id);

    return stmt.step() == SQLITE_DONE;
}

bool GroupRepository::isMember(int group_id, int user_id) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = R"(
        SELECT COUNT(*) FROM group_members WHERE group_id = ? AND user_id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, group_id);
    stmt.bindInt(2, user_id);

    if (stmt.step() == SQLITE_ROW) {
        return stmt.getInt(0) > 0;
    }

    return false;
}

std::string GroupRepository::getMemberRole(int group_id, int user_id) {
    if (!database_ || !database_->isOpen()) return "";

    const std::string sql = R"(
        SELECT role FROM group_members WHERE group_id = ? AND user_id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return "";

    stmt.bindInt(1, group_id);
    stmt.bindInt(2, user_id);

    if (stmt.step() == SQLITE_ROW) {
        return stmt.getText(0);
    }

    return "";
}

int GroupRepository::getMemberCount(int group_id) {
    if (!database_ || !database_->isOpen()) return 0;

    const std::string sql = R"(
        SELECT COUNT(*) FROM group_members WHERE group_id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return 0;

    stmt.bindInt(1, group_id);

    if (stmt.step() == SQLITE_ROW) {
        return stmt.getInt(0);
    }

    return 0;
}

bool GroupRepository::canUserManage(int group_id, int user_id) {
    if (!database_ || !database_->isOpen()) return false;

    // Check if user is creator or admin/moderator
    const std::string sql = R"(
        SELECT COUNT(*) FROM groups g
        LEFT JOIN group_members gm ON g.id = gm.group_id AND gm.user_id = ?
        WHERE g.id = ? AND (g.creator_id = ? OR gm.role IN ('admin', 'moderator'))
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, user_id);
    stmt.bindInt(2, group_id);
    stmt.bindInt(3, user_id);

    if (stmt.step() == SQLITE_ROW) {
        return stmt.getInt(0) > 0;
    }

    return false;
}

} // namespace repositories
} // namespace sohbet
