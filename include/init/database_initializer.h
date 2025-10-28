#pragma once

#include "db/database.h"
#include "db/migration_runner.h"
#include <memory>
#include <string>

namespace sohbet {
namespace init {

/**
 * Database initialization and schema management
 * Ensures database schema exists and is up-to-date on startup
 */
class DatabaseInitializer {
public:
    /**
     * Constructor
     * @param db_path Path to SQLite database file
     * @param migrations_dir Path to directory containing migration SQL files
     */
    DatabaseInitializer(const std::string& db_path, const std::string& migrations_dir);

    /**
     * Initialize database and apply all pending migrations
     * This should be called on application startup
     * @return true if successful, false otherwise
     */
    bool initialize();

    /**
     * Get the database instance
     * @return Shared pointer to Database object
     */
    std::shared_ptr<db::Database> getDatabase() { return database_; }

    /**
     * Get the migration runner instance
     * @return Shared pointer to MigrationRunner object
     */
    std::shared_ptr<db::MigrationRunner> getMigrationRunner() { return migration_runner_; }

    /**
     * Get the last error message
     * @return Error message string
     */
    const std::string& getLastError() const { return last_error_; }

    /**
     * Check if database is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const { return initialized_; }

private:
    std::string db_path_;
    std::string migrations_dir_;
    std::shared_ptr<db::Database> database_;
    std::shared_ptr<db::MigrationRunner> migration_runner_;
    std::string last_error_;
    bool initialized_;

    void setError(const std::string& error) { last_error_ = error; }
};

} // namespace init
} // namespace sohbet
