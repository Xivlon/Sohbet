#include "repositories/user_repository.h"
#include <iostream>
#include <sstream>

namespace sohbet {
namespace repositories {

UserRepository::UserRepository(std::shared_ptr<db::Database> database) : database_(database) {
}

bool UserRepository::migrate() {
    if (!database_ || !database_->isOpen()) {
        return false;
    }
    
    const std::string create_users_table = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            email TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            university TEXT,
            department TEXT,
            enrollment_year INTEGER,
            primary_language TEXT,
            additional_languages TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    return database_->execute(create_users_table);
}

bool UserRepository::createUser(User& user) {
    if (!database_ || !database_->isOpen()) {
        return false;
    }
    
    const std::string sql = R"(
        INSERT INTO users (username, email, password_hash, university, department, 
                          enrollment_year, primary_language, additional_languages)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    )";
    
    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) {
        return false;
    }
    
    // Convert additional languages vector to comma-separated string
    std::string additional_langs;
    const auto& langs = user.getAdditionalLanguages();
    for (size_t i = 0; i < langs.size(); ++i) {
        additional_langs += langs[i];
        if (i < langs.size() - 1) {
            additional_langs += ",";
        }
    }
    
    stmt.bindText(1, user.getUsername());
    stmt.bindText(2, user.getEmail());
    stmt.bindText(3, user.getPasswordHash());
    stmt.bindText(4, user.getUniversity());
    stmt.bindText(5, user.getDepartment());
    
    if (user.getEnrollmentYear().has_value()) {
        stmt.bindInt(6, user.getEnrollmentYear().value());
    } else {
        stmt.bindText(6, ""); // SQLite will treat this as NULL for INTEGER column
    }
    
    stmt.bindText(7, user.getPrimaryLanguage());
    stmt.bindText(8, additional_langs);
    
    int result = stmt.step();
    if (result == SQLITE_DONE) {
        user.setId(static_cast<int>(database_->lastInsertRowId()));
        return true;
    }
    
    return false;
}

std::optional<User> UserRepository::findByUsername(const std::string& username) {
    if (!database_ || !database_->isOpen()) {
        return std::nullopt;
    }
    
    const std::string sql = R"(
        SELECT id, username, email, password_hash, university, department,
               enrollment_year, primary_language, additional_languages
        FROM users WHERE username = ?
    )";
    
    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) {
        return std::nullopt;
    }
    
    stmt.bindText(1, username);
    
    if (stmt.step() == SQLITE_ROW) {
        return userFromStatement(stmt);
    }
    
    return std::nullopt;
}

std::optional<User> UserRepository::findByEmail(const std::string& email) {
    if (!database_ || !database_->isOpen()) {
        return std::nullopt;
    }
    
    const std::string sql = R"(
        SELECT id, username, email, password_hash, university, department,
               enrollment_year, primary_language, additional_languages
        FROM users WHERE email = ?
    )";
    
    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) {
        return std::nullopt;
    }
    
    stmt.bindText(1, email);
    
    if (stmt.step() == SQLITE_ROW) {
        return userFromStatement(stmt);
    }
    
    return std::nullopt;
}

std::optional<User> UserRepository::findById(int id) {
    if (!database_ || !database_->isOpen()) {
        return std::nullopt;
    }
    
    const std::string sql = R"(
        SELECT id, username, email, password_hash, university, department,
               enrollment_year, primary_language, additional_languages
        FROM users WHERE id = ?
    )";
    
    db::Statement stmt(*database_, sql);
    if (!stmt.isValid()) {
        return std::nullopt;
    }
    
    stmt.bindInt(1, id);
    
    if (stmt.step() == SQLITE_ROW) {
        return userFromStatement(stmt);
    }
    
    return std::nullopt;
}

bool UserRepository::usernameExists(const std::string& username) {
    return findByUsername(username).has_value();
}

bool UserRepository::emailExists(const std::string& email) {
    return findByEmail(email).has_value();
}

User UserRepository::userFromStatement(db::Statement& stmt) {
    User user;
    
    user.setId(stmt.getInt(0));
    user.setUsername(stmt.getText(1));
    user.setEmail(stmt.getText(2));
    user.setPasswordHash(stmt.getText(3));
    user.setUniversity(stmt.getText(4));
    user.setDepartment(stmt.getText(5));
    
    // Handle enrollment_year (might be NULL)
    std::string year_str = stmt.getText(6);
    if (!year_str.empty()) {
        user.setEnrollmentYear(std::stoi(year_str));
    }
    
    user.setPrimaryLanguage(stmt.getText(7));
    
    // Parse additional languages from comma-separated string
    std::string additional_langs_str = stmt.getText(8);
    if (!additional_langs_str.empty()) {
        std::vector<std::string> languages;
        std::stringstream ss(additional_langs_str);
        std::string lang;
        
        while (std::getline(ss, lang, ',')) {
            if (!lang.empty()) {
                languages.push_back(lang);
            }
        }
        
        user.setAdditionalLanguages(languages);
    }
    
    return user;
}

} // namespace repositories
} // namespace sohbet