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
    /**
     * Constructor - opens database connection
     * @param db_path Path to database file (use ":memory:" for in-memory database)
     */
    explicit Database(const std::string& db_path);
    
    /**
     * Destructor - closes database connection
     */
    ~Database();
    
    // Disable copy constructor and assignment operator
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    
    // Enable move constructor and assignment operator
    Database(Database&& other) noexcept;
    Database& operator=(Database&& other) noexcept;
    
    /**
     * Execute a SQL statement
     * @param sql The SQL statement to execute
     * @return true if successful, false otherwise
     */
    bool execute(const std::string& sql);
    
    /**
     * Prepare a SQL statement
     * @param sql The SQL statement to prepare
     * @return sqlite3_stmt pointer (caller must finalize)
     */
    sqlite3_stmt* prepare(const std::string& sql);
    
    /**
     * Get the last insert row ID
     * @return The row ID of the last insert operation
     */
    sqlite3_int64 lastInsertRowId() const;
    
    /**
     * Get the last error message
     * @return Error message string
     */
    std::string getLastError() const;
    
    /**
     * Check if database is open
     * @return true if database is open, false otherwise
     */
    bool isOpen() const { return db_ != nullptr; }
    
    /**
     * Get raw SQLite handle (use with caution)
     * @return sqlite3 pointer
     */
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
    /**
     * Constructor
     * @param db Database instance
     * @param sql SQL statement to prepare
     */
    Statement(Database& db, const std::string& sql);
    
    /**
     * Destructor - finalizes statement
     */
    ~Statement();
    
    // Disable copy constructor and assignment operator
    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;
    
    /**
     * Bind integer parameter
     * @param index Parameter index (1-based)
     * @param value Integer value to bind
     * @return true if successful
     */
    bool bindInt(int index, int value);
    
    /**
     * Bind text parameter
     * @param index Parameter index (1-based)
     * @param value Text value to bind
     * @return true if successful
     */
    bool bindText(int index, const std::string& value);
    
    /**
     * Execute the statement
     * @return SQLITE_DONE, SQLITE_ROW, or error code
     */
    int step();
    
    /**
     * Reset the statement for reuse
     * @return true if successful
     */
    bool reset();
    
    /**
     * Get integer column value
     * @param index Column index (0-based)
     * @return Integer value
     */
    int getInt(int index) const;
    
    /**
     * Get text column value
     * @param index Column index (0-based)
     * @return Text value
     */
    std::string getText(int index) const;
    
    /**
     * Check if statement is valid
     * @return true if statement is prepared successfully
     */
    bool isValid() const { return stmt_ != nullptr; }

private:
    sqlite3_stmt* stmt_;
};

} // namespace db
} // namespace sohbet