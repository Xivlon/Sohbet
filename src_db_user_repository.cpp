#include "db/user_repository.h"
#include "db/database.h"
#include "db/password.h"
#include <iostream>
#include <optional>
#include <stdexcept>

namespace sohbet {
namespace db {

UserRepository::UserRepository(Database& db) : db_(db) {}

long long UserRepository::createUser(
    const std::string& id_no,
    const std::string& username,
    const std::string& name,
    const std::string& email,
    const std::string& plaintextPassword,
    const std::string& clearance_level,
    const std::string& institution,
    const std::string& specialization,
    int graduation_year,
    const std::string& phone,
    const std::string& role,
    int account_warnings,
    int bcryptRounds
) {
    if (username.empty() || email.empty() || plaintextPassword.empty()) {
        throw std::invalid_argument("username, email and password are required");
    }

    // Hash password
    std::string hashed = Bcrypt::hashPassword(plaintextPassword, bcryptRounds);

    // Insert row. Use prepared statement via Statement wrapper.
    const std::string sql =
        "INSERT INTO users (id_no, username, name, email, password, clearance_level, institution, specialization, graduation_year, phone, role, account_warnings, created_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, datetime('now'));";

    Statement stmt(db_, sql);

    // Bind parameters (1-based)
    // id_no may be empty => bindNull
    if (id_no.empty()) stmt.bindNull(1); else stmt.bindText(1, id_no);
    stmt.bindText(2, username);
    if (name.empty()) stmt.bindNull(3); else stmt.bindText(3, name);
    stmt.bindText(4, email);
    stmt.bindText(5, hashed);
    if (clearance_level.empty()) stmt.bindNull(6); else stmt.bindText(6, clearance_level);
    if (institution.empty()) stmt.bindNull(7); else stmt.bindText(7, institution);
    if (specialization.empty()) stmt.bindNull(8); else stmt.bindText(8, specialization);
    if (graduation_year <= 0) stmt.bindNull(9); else stmt.bindInt(9, graduation_year);
    if (phone.empty()) stmt.bindNull(10); else stmt.bindText(10, phone);
    stmt.bindText(11, role);
    stmt.bindInt(12, account_warnings);

    int rc = stmt.step();
    if (rc != SQLITE_DONE) {
        std::cerr << "createUser: failed to insert user, sqlite code " << rc << " err: " << db_.getLastError() << std::endl;
        return -1;
    }

    return db_.lastInsertRowId();
}

std::optional<User> UserRepository::findByUsername(const std::string& username) {
    const std::string sql =
        "SELECT id, id_no, username, name, email, clearance_level, institution, specialization, graduation_year, phone, role, account_warnings, created_at "
        "FROM users WHERE username = ? LIMIT 1;";
    Statement stmt(db_, sql);
    stmt.bindText(1, username);

    int rc = stmt.step();
    if (rc == SQLITE_ROW) {
        User u;
        u.id = stmt.getInt64(0);
        u.id_no = stmt.getText(1);
        u.username = stmt.getText(2);
        u.name = stmt.getText(3);
        u.email = stmt.getText(4);
        u.clearance_level = stmt.getText(5);
        u.institution = stmt.getText(6);
        u.specialization = stmt.getText(7);
        u.graduation_year = stmt.isNull(8) ? 0 : stmt.getInt(8);
        u.phone = stmt.getText(9);
        u.role = stmt.getText(10);
        u.account_warnings = stmt.isNull(11) ? 0 : stmt.getInt(11);
        u.created_at = stmt.getText(12);
        return u;
    }

    return std::nullopt;
}

bool UserRepository::authenticate(const std::string& username, const std::string& plaintextPassword) {
    const std::string sql = "SELECT password FROM users WHERE username = ? LIMIT 1;";
    Statement stmt(db_, sql);
    stmt.bindText(1, username);

    int rc = stmt.step();
    if (rc != SQLITE_ROW) {
        // user not found
        return false;
    }

    std::string storedHash = stmt.getText(0);
    try {
        return Bcrypt::verifyPassword(plaintextPassword, storedHash);
    } catch (const std::exception& ex) {
        std::cerr << "authenticate: bcrypt error: " << ex.what() << std::endl;
        return false;
    }
}

} // namespace db
} // namespace sohbet