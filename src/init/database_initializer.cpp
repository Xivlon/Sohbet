#include "init/database_initializer.h"
#include <iostream>
#include <filesystem>

namespace sohbet {
namespace init {

DatabaseInitializer::DatabaseInitializer(const std::string& db_path, const std::string& migrations_dir)
    : db_path_(db_path), migrations_dir_(migrations_dir), initialized_(false) {}

bool DatabaseInitializer::initialize() {
    std::cout << "Initializing database: " << db_path_ << std::endl;

    // Create database instance
    database_ = std::make_shared<db::Database>(db_path_);
    if (!database_->isOpen()) {
        setError("Failed to open database: " + database_->getLastError());
        return false;
    }

    std::cout << "Database opened successfully" << std::endl;

    // Create migration runner
    migration_runner_ = std::make_shared<db::MigrationRunner>(database_);

    // Initialize migration system (creates schema_migrations table)
    if (!migration_runner_->initialize()) {
        setError("Failed to initialize migration system: " + migration_runner_->getLastError());
        return false;
    }

    std::cout << "Migration system initialized" << std::endl;

    // Check if migrations directory exists
    if (!std::filesystem::exists(migrations_dir_)) {
        std::cout << "Warning: Migrations directory not found: " << migrations_dir_ << std::endl;
        std::cout << "Skipping migration loading" << std::endl;
        initialized_ = true;
        return true;
    }

    // Load migrations from directory
    int loaded = migration_runner_->loadMigrationsFromDirectory(migrations_dir_);
    if (loaded < 0) {
        setError("Failed to load migrations: " + migration_runner_->getLastError());
        return false;
    }

    std::cout << "Loaded " << loaded << " migration(s)" << std::endl;

    // Get current version
    int current_version = migration_runner_->getCurrentVersion();
    std::cout << "Current database version: " << current_version << std::endl;

    // Apply pending migrations
    std::cout << "Checking for pending migrations..." << std::endl;
    int applied = migration_runner_->applyPendingMigrations();
    
    if (applied < 0) {
        setError("Failed to apply migrations: " + migration_runner_->getLastError());
        return false;
    }

    if (applied > 0) {
        std::cout << "Applied " << applied << " migration(s)" << std::endl;
        int new_version = migration_runner_->getCurrentVersion();
        std::cout << "Database upgraded to version: " << new_version << std::endl;
    } else {
        std::cout << "Database is up-to-date" << std::endl;
    }

    initialized_ = true;
    return true;
}

} // namespace init
} // namespace sohbet
