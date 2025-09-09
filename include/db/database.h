#pragma once
#include <sqlite3.h>
#include <string>
#include <memory>

namespace sohbet {

class Database {
public:
    explicit Database(const std::string& db_path);
    ~Database();
    
    // Non-copyable, movable
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    Database(Database&&) noexcept;
    Database& operator=(Database&&) noexcept;
    
    // Execute SQL statement
    bool execute(const std::string& sql);
    
    // Get SQLite3 handle for prepared statements
    sqlite3* getHandle() { return db_; }
    
    // Get last error message
    std::string getLastError() const;
    
private:
    sqlite3* db_;
};

} // namespace sohbet