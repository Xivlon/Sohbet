#include "repositories/user_repository.h"
#include "utils/hash.h"
#include <sqlite3.h>

namespace sohbet {

UserRepository::UserRepository(Database& db) : db_(db) {}

bool UserRepository::migrate() {
    const std::string create_table_sql = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            email TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            password_salt TEXT,
            university TEXT,
            department TEXT,
            enrollment_year INTEGER,
            primary_language TEXT,
            additional_languages TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    return db_.execute(create_table_sql);
}

std::optional<User> UserRepository::create(const User& user, const std::string& password) {
    // Hash the password
    std::string password_hash = utils::hash_password(password);
    
    const std::string insert_sql = R"(
        INSERT INTO users (username, email, password_hash, password_salt, university, department, enrollment_year, primary_language)
        VALUES (?, ?, ?, '', ?, ?, ?, ?)
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_.getHandle(), insert_sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return std::nullopt;
    }
    
    sqlite3_bind_text(stmt, 1, user.getUsername().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, user.getEmail().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, password_hash.c_str(), -1, SQLITE_STATIC);
    
    if (user.getUniversity().has_value()) {
        sqlite3_bind_text(stmt, 4, user.getUniversity().value().c_str(), -1, SQLITE_STATIC);
    } else {
        sqlite3_bind_null(stmt, 4);
    }
    
    if (user.getDepartment().has_value()) {
        sqlite3_bind_text(stmt, 5, user.getDepartment().value().c_str(), -1, SQLITE_STATIC);
    } else {
        sqlite3_bind_null(stmt, 5);
    }
    
    if (user.getEnrollmentYear().has_value()) {
        sqlite3_bind_int(stmt, 6, user.getEnrollmentYear().value());
    } else {
        sqlite3_bind_null(stmt, 6);
    }
    
    if (user.getPrimaryLanguage().has_value()) {
        sqlite3_bind_text(stmt, 7, user.getPrimaryLanguage().value().c_str(), -1, SQLITE_STATIC);
    } else {
        sqlite3_bind_null(stmt, 7);
    }
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        return std::nullopt;
    }
    
    // Return the created user with ID
    int user_id = sqlite3_last_insert_rowid(db_.getHandle());
    User created_user = user;
    created_user.setId(user_id);
    created_user.setPasswordHash(password_hash);
    
    return created_user;
}

std::optional<User> UserRepository::findByUsername(const std::string& username) {
    const std::string select_sql = R"(
        SELECT id, username, email, password_hash, university, department, enrollment_year, primary_language
        FROM users WHERE username = ?
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_.getHandle(), select_sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return std::nullopt;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        User user = userFromStatement(stmt);
        sqlite3_finalize(stmt);
        return user;
    }
    
    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::optional<User> UserRepository::findByEmail(const std::string& email) {
    const std::string select_sql = R"(
        SELECT id, username, email, password_hash, university, department, enrollment_year, primary_language
        FROM users WHERE email = ?
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_.getHandle(), select_sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return std::nullopt;
    }
    
    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        User user = userFromStatement(stmt);
        sqlite3_finalize(stmt);
        return user;
    }
    
    sqlite3_finalize(stmt);
    return std::nullopt;
}

bool UserRepository::usernameExists(const std::string& username) {
    return findByUsername(username).has_value();
}

bool UserRepository::emailExists(const std::string& email) {
    return findByEmail(email).has_value();
}

User UserRepository::userFromStatement(sqlite3_stmt* stmt) {
    User user;
    
    user.setId(sqlite3_column_int(stmt, 0));
    user.setUsername(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
    user.setEmail(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
    user.setPasswordHash(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
    
    if (sqlite3_column_type(stmt, 4) != SQLITE_NULL) {
        user.setUniversity(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
    }
    
    if (sqlite3_column_type(stmt, 5) != SQLITE_NULL) {
        user.setDepartment(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
    }
    
    if (sqlite3_column_type(stmt, 6) != SQLITE_NULL) {
        user.setEnrollmentYear(sqlite3_column_int(stmt, 6));
    }
    
    if (sqlite3_column_type(stmt, 7) != SQLITE_NULL) {
        user.setPrimaryLanguage(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7)));
    }
    
    return user;
}

} // namespace sohbet