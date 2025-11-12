#include "repositories/user_repository.h"
#include "utils/hash.h"
#include <sstream>
#include <iostream>

namespace sohbet {
namespace repositories {

UserRepository::UserRepository(std::shared_ptr<db::Database> database)
    : database_(database) {}

// Run migrations (create users table)
bool UserRepository::migrate() {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = R"(
        CREATE TABLE IF NOT EXISTS users (
            id SERIAL PRIMARY KEY,
            username TEXT UNIQUE NOT NULL,
            email TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            name TEXT,
            position TEXT,
            phone_number TEXT,
            university TEXT,
            department TEXT,
            enrollment_year INTEGER,
            warnings INTEGER DEFAULT 0,
            primary_language TEXT,
            additional_languages TEXT,
            role TEXT DEFAULT 'Student',
            avatar_url TEXT,
            banner_url TEXT,
            email_verified INTEGER DEFAULT 0,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    )";

    return database_->execute(sql);
}

// Create a new user
std::optional<User> UserRepository::create(User& user, const std::string& password) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        INSERT INTO users (username, email, password_hash, name, position, phone_number,
                          university, department, enrollment_year, warnings,
                          primary_language, additional_languages)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        RETURNING id
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    // Hash the password securely
    std::string hashed_password = utils::hash_password(password);
    user.setPasswordHash(hashed_password);

    // Convert additional languages vector to comma-separated string
    std::string additional_langs;
    for (size_t i = 0; i < user.getAdditionalLanguages().size(); ++i) {
        additional_langs += user.getAdditionalLanguages()[i];
        if (i < user.getAdditionalLanguages().size() - 1) additional_langs += ",";
    }

    stmt.bindText(1, user.getUsername());
    stmt.bindText(2, user.getEmail());
    stmt.bindText(3, hashed_password);
    stmt.bindText(4, user.getName().has_value() ? user.getName().value() : "");
    stmt.bindText(5, user.getPosition().has_value() ? user.getPosition().value() : "");
    stmt.bindText(6, user.getPhoneNumber().has_value() ? user.getPhoneNumber().value() : "");
    stmt.bindText(7, user.getUniversity().has_value() ? user.getUniversity().value() : "");
    stmt.bindText(8, user.getDepartment().has_value() ? user.getDepartment().value() : "");
    stmt.bindText(9, user.getEnrollmentYear().has_value() ? std::to_string(user.getEnrollmentYear().value()) : "");
    stmt.bindText(10, user.getWarnings().has_value() ? std::to_string(user.getWarnings().value()) : "0");
    stmt.bindText(11, user.getPrimaryLanguage().has_value() ? user.getPrimaryLanguage().value() : "");
    stmt.bindText(12, additional_langs);

    int result = stmt.step();
    if (result == SQLITE_ROW) {
        user.setId(stmt.getInt(0));
        // Call step() again to commit the transaction
        stmt.step();
        return user;
    }

    return std::nullopt;
}

// Find user by username
std::optional<User> UserRepository::findByUsername(const std::string& username) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        SELECT id, username, email, password_hash, name, position, phone_number,
               university, department, enrollment_year, warnings, 
               primary_language, additional_languages, role, avatar_url, banner_url, created_at,
               COALESCE(email_verified, 0) as email_verified
        FROM users WHERE username = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindText(1, username);
    if (stmt.step() == SQLITE_ROW) {
        return userFromStatement(stmt);
    }

    return std::nullopt;
}

// Find user by email
std::optional<User> UserRepository::findByEmail(const std::string& email) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        SELECT id, username, email, password_hash, name, position, phone_number,
               university, department, enrollment_year, warnings,
               primary_language, additional_languages, role, avatar_url, banner_url, created_at,
               COALESCE(email_verified, 0) as email_verified
        FROM users WHERE email = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindText(1, email);
    if (stmt.step() == SQLITE_ROW) {
        return userFromStatement(stmt);
    }

    return std::nullopt;
}

// Find user by ID
std::optional<User> UserRepository::findById(int id) {
    if (!database_ || !database_->isOpen()) return std::nullopt;

    const std::string sql = R"(
        SELECT id, username, email, password_hash, name, position, phone_number,
               university, department, enrollment_year, warnings,
               primary_language, additional_languages, role, avatar_url, banner_url, created_at
        FROM users WHERE id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return std::nullopt;

    stmt.bindInt(1, id);
    if (stmt.step() == SQLITE_ROW) {
        return userFromStatement(stmt);
    }

    return std::nullopt;
}

// Check if username exists
bool UserRepository::usernameExists(const std::string& username) {
    return findByUsername(username).has_value();
}

// Check if email exists
bool UserRepository::emailExists(const std::string& email) {
    return findByEmail(email).has_value();
}

// Find all users with pagination
std::vector<User> UserRepository::findAll(int limit, int offset) {
    std::vector<User> users;
    
    if (!database_ || !database_->isOpen()) return users;

    const std::string sql = R"(
        SELECT id, username, email, password_hash, name, position, phone_number,
               university, department, enrollment_year, warnings,
               primary_language, additional_languages, role, avatar_url, banner_url, created_at
        FROM users
        ORDER BY id
        LIMIT ? OFFSET ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return users;

    stmt.bindInt(1, limit);
    stmt.bindInt(2, offset);

    while (stmt.step() == SQLITE_ROW) {
        users.push_back(userFromStatement(stmt));
    }

    return users;
}

// Count total number of users
int UserRepository::countAll() {
    if (!database_ || !database_->isOpen()) return 0;

    const std::string sql = "SELECT COUNT(*) FROM users";
    db::Statement stmt(*database_, sql);
    
    if (!stmt.isValid()) return 0;
    
    if (stmt.step() == SQLITE_ROW) {
        return stmt.getInt(0);
    }
    
    return 0;
}

// Update user profile
bool UserRepository::update(const User& user) {
    if (!database_ || !database_->isOpen()) return false;

    const std::string sql = R"(
        UPDATE users SET 
            name = ?,
            position = ?,
            phone_number = ?,
            university = ?,
            department = ?,
            enrollment_year = ?,
            primary_language = ?
        WHERE id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindText(1, user.getName().value_or(""));
    stmt.bindText(2, user.getPosition().value_or(""));
    stmt.bindText(3, user.getPhoneNumber().value_or(""));
    stmt.bindText(4, user.getUniversity().value_or(""));
    stmt.bindText(5, user.getDepartment().value_or(""));
    stmt.bindInt(6, user.getEnrollmentYear().value_or(0));
    stmt.bindText(7, user.getPrimaryLanguage().value_or(""));
    stmt.bindInt(8, user.getId().value());

    return stmt.step() == SQLITE_DONE;
}

// Update user password
bool UserRepository::updatePassword(int userId, const std::string& newPassword) {
    if (!database_ || !database_->isOpen()) return false;

    // Hash the new password
    std::string hashed_password = utils::hash_password(newPassword);

    const std::string sql = R"(
        UPDATE users SET password_hash = ? WHERE id = ?
    )";

    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) return false;

    stmt.bindText(1, hashed_password);
    stmt.bindInt(2, userId);

    if (stmt.step() != SQLITE_DONE) {
        return false;
    }

    // Verify that exactly one row was updated
    size_t changes = stmt.affectedRows();
    if (changes == 0) {
        std::cerr << "updatePassword: User ID " << userId << " not found" << std::endl;
        return false;
    } else if (changes != 1) {
        std::cerr << "updatePassword: Expected 1 row updated, got " << changes << std::endl;
        return false;
    }

    return true;
}

// Build User object from a DB row
User UserRepository::userFromStatement(db::Statement& stmt) {
    User user;
    user.setId(stmt.getInt(0));
    user.setUsername(stmt.getText(1));
    user.setEmail(stmt.getText(2));
    user.setPasswordHash(stmt.getText(3));
    
    std::string name = stmt.getText(4);
    if (!name.empty()) user.setName(name);
    
    std::string position = stmt.getText(5);
    if (!position.empty()) user.setPosition(position);
    
    std::string phone = stmt.getText(6);
    if (!phone.empty()) user.setPhoneNumber(phone);
    
    std::string university = stmt.getText(7);
    if (!university.empty()) user.setUniversity(university);
    
    std::string department = stmt.getText(8);
    if (!department.empty()) user.setDepartment(department);

    std::string year_str = stmt.getText(9);
    if (!year_str.empty()) user.setEnrollmentYear(std::stoi(year_str));

    std::string warnings_str = stmt.getText(10);
    if (!warnings_str.empty()) user.setWarnings(std::stoi(warnings_str));

    std::string primary_lang = stmt.getText(11);
    if (!primary_lang.empty()) user.setPrimaryLanguage(primary_lang);

    std::string langs_str = stmt.getText(12);
    if (!langs_str.empty()) {
        std::vector<std::string> langs;
        std::stringstream ss(langs_str);
        std::string lang;
        while (std::getline(ss, lang, ',')) {
            if (!lang.empty()) langs.push_back(lang);
        }
        user.setAdditionalLanguages(langs);
    }

    std::string created = stmt.getText(16);
    if (!created.empty()) user.setCreatedAt(created);

    std::string role = stmt.getText(13);
    if (!role.empty()) user.setRole(role);

    std::string avatar_url = stmt.getText(14);
    if (!avatar_url.empty()) user.setAvatarUrl(avatar_url);

    std::string banner_url = stmt.getText(15);
    if (!banner_url.empty()) user.setBannerUrl(banner_url);

    // email_verified (column 17)
    int email_verified = stmt.getInt(17);
    user.setEmailVerified(email_verified != 0);

    return user;
}

} // namespace repositories
} // namespace sohbet