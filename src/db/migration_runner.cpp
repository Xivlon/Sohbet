#include "db/migration_runner.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <regex>

namespace sohbet {
namespace db {

MigrationRunner::MigrationRunner(std::shared_ptr<Database> database)
    : database_(database) {}

bool MigrationRunner::initialize() {
    clearError();

    if (!database_ || !database_->isOpen()) {
        setError("Database is not open");
        return false;
    }

    // Create schema_migrations table
    const std::string sql = R"(
        CREATE TABLE IF NOT EXISTS schema_migrations (
            version INTEGER PRIMARY KEY,
            name TEXT NOT NULL,
            applied_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";

    bool success = database_->execute(sql);
    if (!success) {
        setError("Failed to create schema_migrations table: " + database_->getLastError());
    }

    return success;
}

int MigrationRunner::getCurrentVersion() {
    if (!database_ || !database_->isOpen()) {
        return 0;
    }

    const std::string sql = "SELECT MAX(version) FROM schema_migrations";
    Statement stmt(*database_, sql);

    if (!stmt.isValid()) {
        return 0;
    }

    if (stmt.step() == SQLITE_ROW) {
        if (!stmt.isNull(0)) {
            return stmt.getInt(0);
        }
    }

    return 0;
}

bool MigrationRunner::isMigrationApplied(int version) {
    if (!database_ || !database_->isOpen()) {
        return false;
    }

    const std::string sql = "SELECT version FROM schema_migrations WHERE version = ?";
    Statement stmt(*database_, sql);

    if (!stmt.isValid()) {
        return false;
    }

    stmt.bindInt(1, version);
    return stmt.step() == SQLITE_ROW;
}

bool MigrationRunner::registerMigration(int version, const std::string& name, const std::string& sql) {
    clearError();

    // Check if version already registered
    for (const auto& m : migrations_) {
        if (m.version == version) {
            setError("Migration version " + std::to_string(version) + " already registered");
            return false;
        }
    }

    Migration migration;
    migration.version = version;
    migration.name = name;
    migration.sql = sql;
    migration.applied_at = "";  // Not yet applied

    migrations_.push_back(migration);

    // Sort migrations by version
    std::sort(migrations_.begin(), migrations_.end(),
              [](const Migration& a, const Migration& b) { return a.version < b.version; });

    return true;
}

bool MigrationRunner::applyMigration(int version) {
    clearError();

    if (!database_ || !database_->isOpen()) {
        setError("Database is not open");
        return false;
    }

    // Find migration with this version
    auto it = std::find_if(migrations_.begin(), migrations_.end(),
                           [version](const Migration& m) { return m.version == version; });

    if (it == migrations_.end()) {
        setError("Migration version " + std::to_string(version) + " not found");
        return false;
    }

    // Check if already applied
    if (isMigrationApplied(version)) {
        setError("Migration version " + std::to_string(version) + " already applied");
        return false;
    }

    std::cout << "Applying migration " << version << ": " << it->name << std::endl;

    // Execute migration SQL
    bool success = database_->execute(it->sql);
    if (!success) {
        setError("Failed to execute migration " + std::to_string(version) + ": " + database_->getLastError());
        return false;
    }

    // Record migration as applied
    success = recordMigration(version, it->name);
    if (!success) {
        setError("Failed to record migration " + std::to_string(version) + " as applied");
        return false;
    }

    std::cout << "Migration " << version << " applied successfully" << std::endl;
    return true;
}

int MigrationRunner::applyPendingMigrations() {
    clearError();

    if (!database_ || !database_->isOpen()) {
        setError("Database is not open");
        return -1;
    }

    auto pending = getPendingMigrations();
    if (pending.empty()) {
        std::cout << "No pending migrations to apply" << std::endl;
        return 0;
    }

    int applied_count = 0;
    for (const auto& migration : pending) {
        if (applyMigration(migration.version)) {
            applied_count++;
        } else {
            // Stop on first error
            setError("Failed to apply migration " + std::to_string(migration.version) + ": " + last_error_);
            return -1;
        }
    }

    return applied_count;
}

int MigrationRunner::loadMigrationsFromDirectory(const std::string& directory_path) {
    clearError();

    if (!std::filesystem::exists(directory_path)) {
        setError("Migration directory does not exist: " + directory_path);
        return -1;
    }

    if (!std::filesystem::is_directory(directory_path)) {
        setError("Path is not a directory: " + directory_path);
        return -1;
    }

    int loaded_count = 0;
    std::regex migration_pattern(R"((\d+)_(.+)\.sql)");

    // Iterate through files in directory
    for (const auto& entry : std::filesystem::directory_iterator(directory_path)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        std::string filename = entry.path().filename().string();
        std::smatch match;

        if (std::regex_match(filename, match, migration_pattern)) {
            int version = std::stoi(match[1].str());
            std::string name = match[2].str();

            // Read SQL file content
            std::ifstream file(entry.path());
            if (!file.is_open()) {
                setError("Failed to open migration file: " + entry.path().string());
                continue;
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string sql = buffer.str();

            if (registerMigration(version, name, sql)) {
                loaded_count++;
                std::cout << "Loaded migration " << version << ": " << name << std::endl;
            }
        }
    }

    return loaded_count;
}

std::vector<Migration> MigrationRunner::getAllMigrations() {
    std::vector<Migration> result = migrations_;

    // Mark which ones are applied
    for (auto& migration : result) {
        if (isMigrationApplied(migration.version)) {
            // Get applied_at timestamp
            const std::string sql = "SELECT applied_at FROM schema_migrations WHERE version = ?";
            Statement stmt(*database_, sql);
            if (stmt.isValid()) {
                stmt.bindInt(1, migration.version);
                if (stmt.step() == SQLITE_ROW) {
                    migration.applied_at = stmt.getText(0);
                }
            }
        }
    }

    return result;
}

std::vector<Migration> MigrationRunner::getPendingMigrations() {
    std::vector<Migration> pending;

    for (const auto& migration : migrations_) {
        if (!isMigrationApplied(migration.version)) {
            pending.push_back(migration);
        }
    }

    return pending;
}

bool MigrationRunner::recordMigration(int version, const std::string& name) {
    const std::string sql = "INSERT INTO schema_migrations (version, name) VALUES (?, ?)";
    Statement stmt(*database_, sql);

    if (!stmt.isValid()) {
        return false;
    }

    stmt.bindInt(1, version);
    stmt.bindText(2, name);

    return stmt.step() == SQLITE_DONE;
}

} // namespace db
} // namespace sohbet
