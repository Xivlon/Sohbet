#pragma once

#include <sqlite3.h>
#include <string>
#include <memory>
#include <vector>

namespace sohbet {
namespace db {

/**
 * RAII wrapper for SQLite database connections
 */
class Database {
public:
    explicit Database(const std::string& db_path);
    ~Database();

    // Non-copyable, movable
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    Database(Database&& other) noexcept;
    Database& operator=(Database&& other) noexcept;

    // Execute SQL statement
    bool execute(const std::string& sql);

    // Prepare a SQL statement
    sqlite3_stmt* prepare(const std::string& sql);

    // Get last insert row ID
    sqlite3_int64 lastInsertRowId() const;

    // Get last error message
    std::string getLastError() const;

    // Check if database is open
    bool isOpen() const { return db_ != nullptr; }

    // Get raw SQLite handle
    sqlite3* getHandle() const { return db_; }

private:
    sqlite3* db_;
    std::string db_path_;

    void close();
};

/**
 * RAII wrapper for SQLite prepared statements
 */
class Statement {
public:
    Statement(Database& db, const std::string& sql);
    ~Statement();

    // Non-copyable
    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;

    // Bind parameters
    bool bindInt(int index, int value);
    bool bindText(int index, const std::string& value);
    bool bindNull(int index);

    // Execution
    int step();
    bool reset();

    // Column getters
    int getInt(int index) const;
    sqlite3_int64 getInt64(int index) const;
    std::string getText(int index) const;
    bool isNull(int index) const;

    bool isValid() const { return stmt_ != nullptr; }

private:
    sqlite3_stmt* stmt_;
};

} // namespace db
} // namespace sohbet
