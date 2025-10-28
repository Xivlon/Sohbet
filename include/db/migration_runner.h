#pragma once

#include "db/database.h"
#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace sohbet {
namespace db {

/**
 * Migration information structure
 */
struct Migration {
    int version;
    std::string name;
    std::string sql;
    std::string applied_at;  // Empty if not yet applied
};

/**
 * Database migration runner
 * Manages schema versioning and SQL migration execution
 */
class MigrationRunner {
public:
    /**
     * Constructor
     * @param database Database instance
     */
    explicit MigrationRunner(std::shared_ptr<Database> database);

    /**
     * Initialize the migration system
     * Creates the schema_migrations table if it doesn't exist
     * @return true if successful, false otherwise
     */
    bool initialize();

    /**
     * Get the current database schema version
     * @return Current version number, or 0 if no migrations applied
     */
    int getCurrentVersion();

    /**
     * Check if a specific migration version has been applied
     * @param version Migration version number
     * @return true if applied, false otherwise
     */
    bool isMigrationApplied(int version);

    /**
     * Register a migration (does not apply it yet)
     * @param version Migration version number (must be unique)
     * @param name Migration name/description
     * @param sql SQL statements to execute
     * @return true if registered successfully, false if version already exists
     */
    bool registerMigration(int version, const std::string& name, const std::string& sql);

    /**
     * Apply a specific migration
     * @param version Migration version number
     * @return true if successful, false otherwise
     */
    bool applyMigration(int version);

    /**
     * Apply all pending migrations (migrations that haven't been applied yet)
     * @return Number of migrations applied, or -1 on error
     */
    int applyPendingMigrations();

    /**
     * Load migrations from SQL files in a directory
     * Files should be named like: 001_migration_name.sql, 002_another_migration.sql
     * @param directory_path Path to directory containing migration files
     * @return Number of migrations loaded, or -1 on error
     */
    int loadMigrationsFromDirectory(const std::string& directory_path);

    /**
     * Get list of all registered migrations
     * @return Vector of Migration objects
     */
    std::vector<Migration> getAllMigrations();

    /**
     * Get list of pending (not yet applied) migrations
     * @return Vector of Migration objects
     */
    std::vector<Migration> getPendingMigrations();

    /**
     * Get the last error message
     * @return Error message string
     */
    const std::string& getLastError() const { return last_error_; }

private:
    std::shared_ptr<Database> database_;
    std::vector<Migration> migrations_;
    std::string last_error_;

    void setError(const std::string& error) { last_error_ = error; }
    void clearError() { last_error_.clear(); }

    // Helper to record a migration as applied in the database
    bool recordMigration(int version, const std::string& name);
};

} // namespace db
} // namespace sohbet
