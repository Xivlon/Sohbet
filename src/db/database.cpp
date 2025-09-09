#include "db/database.h"
#include <iostream>

namespace sohbet {
namespace db {

Database::Database(const std::string& db_path) : db_(nullptr), db_path_(db_path) {
    int result = sqlite3_open(db_path.c_str(), &db_);
    if (result != SQLITE_OK) {
        std::cerr << "Failed to open database: " << sqlite3_errmsg(db_) << std::endl;
        if (db_) {
            sqlite3_close(db_);
            db_ = nullptr;
        }
    }
}

Database::~Database() {
    close();
}

Database::Database(Database&& other) noexcept : db_(other.db_), db_path_(std::move(other.db_path_)) {
    other.db_ = nullptr;
}

Database& Database::operator=(Database&& other) noexcept {
    if (this != &other) {
        close();
        db_ = other.db_;
        db_path_ = std::move(other.db_path_);
        other.db_ = nullptr;
    }
    return *this;
}

bool Database::execute(const std::string& sql) {
    if (!db_) return false;
    
    char* error_msg = nullptr;
    int result = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &error_msg);
    
    if (result != SQLITE_OK) {
        std::cerr << "SQL error: " << error_msg << std::endl;
        sqlite3_free(error_msg);
        return false;
    }
    
    return true;
}

sqlite3_stmt* Database::prepare(const std::string& sql) {
    if (!db_) return nullptr;
    
    sqlite3_stmt* stmt = nullptr;
    int result = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    
    if (result != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
        return nullptr;
    }
    
    return stmt;
}

sqlite3_int64 Database::lastInsertRowId() const {
    if (!db_) return -1;
    return sqlite3_last_insert_rowid(db_);
}

std::string Database::getLastError() const {
    if (!db_) return "Database not open";
    return sqlite3_errmsg(db_);
}

void Database::close() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

// Statement implementation

Statement::Statement(Database& db, const std::string& sql) : stmt_(nullptr) {
    stmt_ = db.prepare(sql);
}

Statement::~Statement() {
    if (stmt_) {
        sqlite3_finalize(stmt_);
    }
}

bool Statement::bindInt(int index, int value) {
    if (!stmt_) return false;
    return sqlite3_bind_int(stmt_, index, value) == SQLITE_OK;
}

bool Statement::bindText(int index, const std::string& value) {
    if (!stmt_) return false;
    return sqlite3_bind_text(stmt_, index, value.c_str(), -1, SQLITE_TRANSIENT) == SQLITE_OK;
}

int Statement::step() {
    if (!stmt_) return SQLITE_ERROR;
    return sqlite3_step(stmt_);
}

bool Statement::reset() {
    if (!stmt_) return false;
    return sqlite3_reset(stmt_) == SQLITE_OK;
}

int Statement::getInt(int index) const {
    if (!stmt_) return 0;
    return sqlite3_column_int(stmt_, index);
}

std::string Statement::getText(int index) const {
    if (!stmt_) return "";
    const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt_, index));
    return text ? text : "";
}

} // namespace db
} // namespace sohbet