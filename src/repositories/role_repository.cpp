#include "repositories/role_repository.h"
#include <iostream>

namespace sohbet {
namespace repositories {

RoleRepository::RoleRepository(std::shared_ptr<db::Database> database)
    : database_(database) {}

std::optional<Role> RoleRepository::findById(int id) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        SELECT id, name, description, created_at
        FROM roles WHERE id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindInt(1, id);
    
    if (stmt.step() == SQLITE_ROW) {
        Role role;
        role.setId(stmt.getInt(0));
        role.setName(stmt.getText(1));
        role.setDescription(stmt.getText(2));
        role.setCreatedAt(stmt.getText(3));
        return role;
    }

    return std::nullopt;
}

std::optional<Role> RoleRepository::findByName(const std::string& name) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        SELECT id, name, description, created_at
        FROM roles WHERE name = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindText(1, name);
    
    if (stmt.step() == SQLITE_ROW) {
        Role role;
        role.setId(stmt.getInt(0));
        role.setName(stmt.getText(1));
        role.setDescription(stmt.getText(2));
        role.setCreatedAt(stmt.getText(3));
        return role;
    }

    return std::nullopt;
}

std::vector<Role> RoleRepository::findAll() {
    std::vector<Role> roles;
    if (!database_ || !database_->isOpen()) return roles;

    const std::string sql = R"(
        SELECT id, name, description, created_at
        FROM roles ORDER BY name
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return roles;

    while (stmt.step() == SQLITE_ROW) {
        Role role;
        role.setId(stmt.getInt(0));
        role.setName(stmt.getText(1));
        role.setDescription(stmt.getText(2));
        role.setCreatedAt(stmt.getText(3));
        roles.push_back(role);
    }

    return roles;
}

std::vector<std::string> RoleRepository::getRolePermissions(int role_id) {
    std::vector<std::string> permissions;
    if (!database_ || !database_->isOpen()) return permissions;

    const std::string sql = R"(
        SELECT permission FROM role_permissions WHERE role_id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return permissions;

    stmt.bindInt(1, role_id);

    while (stmt.step() == SQLITE_ROW) {
        permissions.push_back(stmt.getText(0));
    }

    return permissions;
}

bool RoleRepository::hasPermission(int role_id, const std::string& permission) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = R"(
        SELECT COUNT(*) FROM role_permissions 
        WHERE role_id = ? AND permission = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, role_id);
    stmt.bindText(2, permission);

    if (stmt.step() == SQLITE_ROW) {
        return stmt.getInt(0) > 0;
    }

    return false;
}

std::optional<Role> RoleRepository::getUserRole(int user_id) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        SELECT r.id, r.name, r.description, r.created_at
        FROM roles r
        INNER JOIN user_roles ur ON r.id = ur.role_id
        WHERE ur.user_id = ?
        LIMIT 1
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindInt(1, user_id);
    
    if (stmt.step() == SQLITE_ROW) {
        Role role;
        role.setId(stmt.getInt(0));
        role.setName(stmt.getText(1));
        role.setDescription(stmt.getText(2));
        role.setCreatedAt(stmt.getText(3));
        return role;
    }

    return std::nullopt;
}

bool RoleRepository::userHasPermission(int user_id, const std::string& permission) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = R"(
        SELECT COUNT(*) FROM role_permissions rp
        INNER JOIN user_roles ur ON rp.role_id = ur.role_id
        WHERE ur.user_id = ? AND rp.permission = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, user_id);
    stmt.bindText(2, permission);

    if (stmt.step() == SQLITE_ROW) {
        return stmt.getInt(0) > 0;
    }

    return false;
}

bool RoleRepository::assignRoleToUser(int user_id, int role_id) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = R"(
        INSERT OR IGNORE INTO user_roles (user_id, role_id)
        VALUES (?, ?)
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, user_id);
    stmt.bindInt(2, role_id);

    return stmt.step() == SQLITE_DONE;
}

} // namespace repositories
} // namespace sohbet
