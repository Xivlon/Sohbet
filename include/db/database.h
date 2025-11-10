#pragma once

#include <pqxx/pqxx>
#include <string>
#include <memory>
#include <vector>

// SQLite compatibility constants (defined globally for backward compatibility)
constexpr int SQLITE_ROW = 100;
constexpr int SQLITE_DONE = 101;
constexpr int SQLITE_OK = 0;
constexpr int SQLITE_ERROR = 1;

namespace sohbet {
namespace db {

/**
 * RAII wrapper for PostgreSQL database connections
 */
class Database {
public:
    explicit Database(const std::string& connection_string);
    ~Database();

    // Non-copyable, movable
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    Database(Database&& other) noexcept;
    Database& operator=(Database&& other) noexcept;

    // Friend declaration for Statement to access private members
    friend class Statement;

    // Execute SQL statement
    bool execute(const std::string& sql);

    // Prepare a SQL statement (returns a unique identifier for the prepared statement)
    std::string prepare(const std::string& sql);

    // Get last insert row ID
    long long lastInsertRowId() const;

    // Get last error message
    std::string getLastError() const;

    // Check if database is open
    bool isOpen() const { return conn_ != nullptr && conn_->is_open(); }

    // Get raw PostgreSQL connection handle
    pqxx::connection* getHandle() const { return conn_.get(); }

private:
    std::unique_ptr<pqxx::connection> conn_;
    std::string connection_string_;
    mutable std::string last_error_;
    mutable long long last_insert_id_;

    void close();
};

/**
 * RAII wrapper for PostgreSQL prepared statements
 */
class Statement {
public:
    Statement(Database& db, const std::string& sql);
    ~Statement();

    // Non-copyable
    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;

    // Bind parameters (1-indexed like SQLite)
    bool bindInt(int index, int value);
    bool bindDouble(int index, double value);
    bool bindText(int index, const std::string& value);
    bool bindNull(int index);

    // Execution
    int step();  // Returns SQLITE_ROW (100), SQLITE_DONE (101), or SQLITE_ERROR
    bool reset();

    // Column getters (0-indexed)
    int getInt(int index) const;
    long long getInt64(int index) const;
    double getDouble(int index) const;
    std::string getText(int index) const;
    bool isNull(int index) const;

    // Get number of affected rows (for UPDATE/DELETE/INSERT)
    size_t affectedRows() const;

    bool isValid() const { return work_ != nullptr; }

private:
    Database& db_;
    std::unique_ptr<pqxx::work> work_;
    std::string sql_;
    std::vector<std::string> params_;
    pqxx::result result_;
    size_t current_row_;
    bool executed_;
    bool done_;
};

} // namespace db
} // namespace sohbet
