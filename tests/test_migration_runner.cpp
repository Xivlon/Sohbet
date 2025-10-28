#include "db/migration_runner.h"
#include "db/database.h"
#include <iostream>
#include <cassert>
#include <memory>
#include <filesystem>
#include <fstream>
#include <unistd.h>  // for getpid()

void testMigrationInitialization() {
    std::cout << "Testing migration initialization..." << std::endl;

    auto db = std::make_shared<sohbet::db::Database>(":memory:");
    assert(db->isOpen());

    sohbet::db::MigrationRunner runner(db);
    assert(runner.initialize());

    // Check that schema_migrations table was created
    const std::string sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='schema_migrations'";
    sohbet::db::Statement stmt(*db, sql);
    assert(stmt.isValid());
    assert(stmt.step() == SQLITE_ROW);

    std::cout << "Migration initialization test passed!" << std::endl;
}

void testMigrationRegistration() {
    std::cout << "Testing migration registration..." << std::endl;

    auto db = std::make_shared<sohbet::db::Database>(":memory:");
    assert(db->isOpen());

    sohbet::db::MigrationRunner runner(db);
    assert(runner.initialize());

    // Register some migrations
    assert(runner.registerMigration(1, "first_migration", "CREATE TABLE test1 (id INTEGER)"));
    assert(runner.registerMigration(2, "second_migration", "CREATE TABLE test2 (id INTEGER)"));
    assert(runner.registerMigration(3, "third_migration", "CREATE TABLE test3 (id INTEGER)"));

    // Try to register duplicate version
    assert(!runner.registerMigration(2, "duplicate", "SELECT 1"));

    // Check all migrations are registered
    auto all_migrations = runner.getAllMigrations();
    assert(all_migrations.size() == 3);
    assert(all_migrations[0].version == 1);
    assert(all_migrations[1].version == 2);
    assert(all_migrations[2].version == 3);

    std::cout << "Migration registration test passed!" << std::endl;
}

void testMigrationApplication() {
    std::cout << "Testing migration application..." << std::endl;

    auto db = std::make_shared<sohbet::db::Database>(":memory:");
    assert(db->isOpen());

    sohbet::db::MigrationRunner runner(db);
    assert(runner.initialize());

    // Register a migration
    assert(runner.registerMigration(1, "create_users", 
        "CREATE TABLE users (id INTEGER PRIMARY KEY, username TEXT)"));

    // Check current version
    assert(runner.getCurrentVersion() == 0);

    // Apply the migration
    assert(runner.applyMigration(1));

    // Check version is updated
    assert(runner.getCurrentVersion() == 1);

    // Check migration is marked as applied
    assert(runner.isMigrationApplied(1));

    // Check that the table was created
    const std::string sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='users'";
    sohbet::db::Statement stmt(*db, sql);
    assert(stmt.isValid());
    assert(stmt.step() == SQLITE_ROW);

    // Try to apply the same migration again
    assert(!runner.applyMigration(1));

    std::cout << "Migration application test passed!" << std::endl;
}

void testPendingMigrations() {
    std::cout << "Testing pending migrations..." << std::endl;

    auto db = std::make_shared<sohbet::db::Database>(":memory:");
    assert(db->isOpen());

    sohbet::db::MigrationRunner runner(db);
    assert(runner.initialize());

    // Register multiple migrations
    assert(runner.registerMigration(1, "migration_1", "CREATE TABLE t1 (id INTEGER)"));
    assert(runner.registerMigration(2, "migration_2", "CREATE TABLE t2 (id INTEGER)"));
    assert(runner.registerMigration(3, "migration_3", "CREATE TABLE t3 (id INTEGER)"));

    // All should be pending
    auto pending = runner.getPendingMigrations();
    assert(pending.size() == 3);

    // Apply all pending migrations
    int applied = runner.applyPendingMigrations();
    assert(applied == 3);

    // No more pending migrations
    pending = runner.getPendingMigrations();
    assert(pending.empty());

    // Check version
    assert(runner.getCurrentVersion() == 3);

    std::cout << "Pending migrations test passed!" << std::endl;
}

void testLoadMigrationsFromDirectory() {
    std::cout << "Testing loading migrations from directory..." << std::endl;

    auto db = std::make_shared<sohbet::db::Database>(":memory:");
    assert(db->isOpen());

    sohbet::db::MigrationRunner runner(db);
    assert(runner.initialize());

    // Create a temporary directory for test migrations
    std::string temp_dir = "/tmp/test_migrations_" + std::to_string(getpid());
    std::filesystem::create_directory(temp_dir);

    // Create some migration files
    std::ofstream m1(temp_dir + "/001_first.sql");
    m1 << "CREATE TABLE test_table_1 (id INTEGER);";
    m1.close();

    std::ofstream m2(temp_dir + "/002_second.sql");
    m2 << "CREATE TABLE test_table_2 (id INTEGER);";
    m2.close();

    // Load migrations
    int loaded = runner.loadMigrationsFromDirectory(temp_dir);
    assert(loaded == 2);

    auto all_migrations = runner.getAllMigrations();
    assert(all_migrations.size() == 2);

    // Apply all loaded migrations
    int applied = runner.applyPendingMigrations();
    assert(applied == 2);

    // Clean up
    std::filesystem::remove_all(temp_dir);

    std::cout << "Loading migrations from directory test passed!" << std::endl;
}

int main() {
    std::cout << "Running Migration Runner tests..." << std::endl;

    try {
        testMigrationInitialization();
        testMigrationRegistration();
        testMigrationApplication();
        testPendingMigrations();
        testLoadMigrationsFromDirectory();

        std::cout << "\nAll Migration Runner tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
