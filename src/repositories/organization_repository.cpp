#include "repositories/organization_repository.h"
#include <iostream>

namespace sohbet {
namespace repositories {

OrganizationRepository::OrganizationRepository(std::shared_ptr<db::Database> database)
    : database_(database) {}

std::optional<Organization> OrganizationRepository::create(Organization& org) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        INSERT INTO organizations (name, type, description, email, website, logo_url)
        VALUES (?, ?, ?, ?, ?, ?)
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindText(1, org.getName());
    stmt.bindText(2, org.getType());
    if (org.getDescription().has_value()) {
        stmt.bindText(3, org.getDescription().value());
    } else {
        stmt.bindNull(3);
    }
    if (org.getEmail().has_value()) {
        stmt.bindText(4, org.getEmail().value());
    } else {
        stmt.bindNull(4);
    }
    if (org.getWebsite().has_value()) {
        stmt.bindText(5, org.getWebsite().value());
    } else {
        stmt.bindNull(5);
    }
    if (org.getLogoUrl().has_value()) {
        stmt.bindText(6, org.getLogoUrl().value());
    } else {
        stmt.bindNull(6);
    }

    int result = stmt.step();
    if (result == SQLITE_DONE) {
        org.setId(static_cast<int>(database_->lastInsertRowId()));
        return org;
    }

    return std::nullopt;
}

std::optional<Organization> OrganizationRepository::findById(int id) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        SELECT id, name, type, description, email, website, logo_url, created_at, updated_at
        FROM organizations WHERE id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindInt(1, id);

    if (stmt.step() == SQLITE_ROW) {
        Organization org;
        org.setId(stmt.getInt(0));
        org.setName(stmt.getText(1));
        org.setType(stmt.getText(2));
        if (!stmt.isNull(3)) {
            org.setDescription(stmt.getText(3));
        }
        if (!stmt.isNull(4)) {
            org.setEmail(stmt.getText(4));
        }
        if (!stmt.isNull(5)) {
            org.setWebsite(stmt.getText(5));
        }
        if (!stmt.isNull(6)) {
            org.setLogoUrl(stmt.getText(6));
        }
        org.setCreatedAt(stmt.getText(7));
        org.setUpdatedAt(stmt.getText(8));
        return org;
    }

    return std::nullopt;
}

std::optional<Organization> OrganizationRepository::findByName(const std::string& name) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        SELECT id, name, type, description, email, website, logo_url, created_at, updated_at
        FROM organizations WHERE name = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindText(1, name);

    if (stmt.step() == SQLITE_ROW) {
        Organization org;
        org.setId(stmt.getInt(0));
        org.setName(stmt.getText(1));
        org.setType(stmt.getText(2));
        if (!stmt.isNull(3)) {
            org.setDescription(stmt.getText(3));
        }
        if (!stmt.isNull(4)) {
            org.setEmail(stmt.getText(4));
        }
        if (!stmt.isNull(5)) {
            org.setWebsite(stmt.getText(5));
        }
        if (!stmt.isNull(6)) {
            org.setLogoUrl(stmt.getText(6));
        }
        org.setCreatedAt(stmt.getText(7));
        org.setUpdatedAt(stmt.getText(8));
        return org;
    }

    return std::nullopt;
}

std::vector<Organization> OrganizationRepository::findAll(int limit, int offset) {
    std::vector<Organization> orgs;
    if (!database_ || !database_->isOpen()) return orgs;

    const std::string sql = R"(
        SELECT id, name, type, description, email, website, logo_url, created_at, updated_at
        FROM organizations
        ORDER BY name ASC
        LIMIT ? OFFSET ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return orgs;

    stmt.bindInt(1, limit);
    stmt.bindInt(2, offset);

    while (stmt.step() == SQLITE_ROW) {
        Organization org;
        org.setId(stmt.getInt(0));
        org.setName(stmt.getText(1));
        org.setType(stmt.getText(2));
        if (!stmt.isNull(3)) {
            org.setDescription(stmt.getText(3));
        }
        if (!stmt.isNull(4)) {
            org.setEmail(stmt.getText(4));
        }
        if (!stmt.isNull(5)) {
            org.setWebsite(stmt.getText(5));
        }
        if (!stmt.isNull(6)) {
            org.setLogoUrl(stmt.getText(6));
        }
        org.setCreatedAt(stmt.getText(7));
        org.setUpdatedAt(stmt.getText(8));
        orgs.push_back(org);
    }

    return orgs;
}

std::vector<Organization> OrganizationRepository::findByType(const std::string& type) {
    std::vector<Organization> orgs;
    if (!database_ || !database_->isOpen()) return orgs;

    const std::string sql = R"(
        SELECT id, name, type, description, email, website, logo_url, created_at, updated_at
        FROM organizations
        WHERE type = ?
        ORDER BY name ASC
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return orgs;

    stmt.bindText(1, type);

    while (stmt.step() == SQLITE_ROW) {
        Organization org;
        org.setId(stmt.getInt(0));
        org.setName(stmt.getText(1));
        org.setType(stmt.getText(2));
        if (!stmt.isNull(3)) {
            org.setDescription(stmt.getText(3));
        }
        if (!stmt.isNull(4)) {
            org.setEmail(stmt.getText(4));
        }
        if (!stmt.isNull(5)) {
            org.setWebsite(stmt.getText(5));
        }
        if (!stmt.isNull(6)) {
            org.setLogoUrl(stmt.getText(6));
        }
        org.setCreatedAt(stmt.getText(7));
        org.setUpdatedAt(stmt.getText(8));
        orgs.push_back(org);
    }

    return orgs;
}

bool OrganizationRepository::update(const Organization& org) {
    if (!database_ || !database_->isOpen()) return false;
    if (!org.getId().has_value()) return false;

    const std::string sql = R"(
        UPDATE organizations 
        SET name = ?, type = ?, description = ?, email = ?, website = ?, 
            logo_url = ?, updated_at = CURRENT_TIMESTAMP
        WHERE id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindText(1, org.getName());
    stmt.bindText(2, org.getType());
    if (org.getDescription().has_value()) {
        stmt.bindText(3, org.getDescription().value());
    } else {
        stmt.bindNull(3);
    }
    if (org.getEmail().has_value()) {
        stmt.bindText(4, org.getEmail().value());
    } else {
        stmt.bindNull(4);
    }
    if (org.getWebsite().has_value()) {
        stmt.bindText(5, org.getWebsite().value());
    } else {
        stmt.bindNull(5);
    }
    if (org.getLogoUrl().has_value()) {
        stmt.bindText(6, org.getLogoUrl().value());
    } else {
        stmt.bindNull(6);
    }
    stmt.bindInt(7, org.getId().value());

    return stmt.step() == SQLITE_DONE;
}

bool OrganizationRepository::deleteById(int id) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = "DELETE FROM organizations WHERE id = ?";
    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, id);
    return stmt.step() == SQLITE_DONE;
}

bool OrganizationRepository::addAccount(int org_id, int user_id, const std::string& role) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = R"(
        INSERT OR IGNORE INTO organization_accounts (organization_id, user_id, role)
        VALUES (?, ?, ?)
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, org_id);
    stmt.bindInt(2, user_id);
    stmt.bindText(3, role);

    return stmt.step() == SQLITE_DONE;
}

bool OrganizationRepository::removeAccount(int org_id, int user_id) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = R"(
        DELETE FROM organization_accounts WHERE organization_id = ? AND user_id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, org_id);
    stmt.bindInt(2, user_id);

    return stmt.step() == SQLITE_DONE;
}

bool OrganizationRepository::updateAccountRole(int org_id, int user_id, const std::string& role) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = R"(
        UPDATE organization_accounts SET role = ? WHERE organization_id = ? AND user_id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindText(1, role);
    stmt.bindInt(2, org_id);
    stmt.bindInt(3, user_id);

    return stmt.step() == SQLITE_DONE;
}

bool OrganizationRepository::isAccountManager(int org_id, int user_id) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = R"(
        SELECT COUNT(*) FROM organization_accounts WHERE organization_id = ? AND user_id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, org_id);
    stmt.bindInt(2, user_id);

    if (stmt.step() == SQLITE_ROW) {
        return stmt.getInt(0) > 0;
    }

    return false;
}

std::string OrganizationRepository::getAccountRole(int org_id, int user_id) {
    if (!database_ || !database_->isOpen()) return "";

    const std::string sql = R"(
        SELECT role FROM organization_accounts WHERE organization_id = ? AND user_id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return "";

    stmt.bindInt(1, org_id);
    stmt.bindInt(2, user_id);

    if (stmt.step() == SQLITE_ROW) {
        return stmt.getText(0);
    }

    return "";
}

bool OrganizationRepository::canUserManage(int org_id, int user_id) {
    if (!database_ || !database_->isOpen()) return false;

    // Check if user is owner or admin
    const std::string sql = R"(
        SELECT COUNT(*) FROM organization_accounts
        WHERE organization_id = ? AND user_id = ? AND role IN ('owner', 'admin')
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, org_id);
    stmt.bindInt(2, user_id);

    if (stmt.step() == SQLITE_ROW) {
        return stmt.getInt(0) > 0;
    }

    return false;
}

} // namespace repositories
} // namespace sohbet
