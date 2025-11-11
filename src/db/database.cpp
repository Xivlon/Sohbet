#include "db/database.h"
#include <stdexcept>
#include <iostream>
#include <sstream>

namespace sohbet {
namespace db {

Database::Database(const std::string& connection_string)
    : conn_(nullptr), connection_string_(connection_string), last_insert_id_(0) {
    try {
        conn_ = std::make_unique<pqxx::connection>(connection_string);
        if (!conn_->is_open()) {
            throw std::runtime_error("Failed to open PostgreSQL connection");
        }
    } catch (const std::exception& e) {
        last_error_ = e.what();
        conn_ = nullptr;
        throw std::runtime_error("Failed to connect to database: " + std::string(e.what()));
    }
}

Database::~Database() {
    close();
}

Database::Database(Database&& other) noexcept
    : conn_(std::move(other.conn_)),
      connection_string_(std::move(other.connection_string_)),
      last_error_(std::move(other.last_error_)),
      last_insert_id_(other.last_insert_id_) {
}

Database& Database::operator=(Database&& other) noexcept {
    if (this != &other) {
        close();
        conn_ = std::move(other.conn_);
        connection_string_ = std::move(other.connection_string_);
        last_error_ = std::move(other.last_error_);
        last_insert_id_ = other.last_insert_id_;
    }
    return *this;
}

bool Database::execute(const std::string& sql) {
    if (!conn_ || !conn_->is_open()) {
        last_error_ = "Database not open";
        return false;
    }

    try {
        pqxx::work txn(*conn_);
        txn.exec(sql);
        txn.commit();
        return true;
    } catch (const std::exception& e) {
        last_error_ = e.what();
        std::cerr << "SQL error: " << e.what() << std::endl;
        return false;
    }
}

std::string Database::prepare(const std::string& sql) {
    // PostgreSQL doesn't require explicit preparation in the same way SQLite does
    // We'll return the SQL itself as the "prepared statement identifier"
    return sql;
}

long long Database::lastInsertRowId() const {
    return last_insert_id_;
}

std::string Database::getLastError() const {
    return conn_ && conn_->is_open() ? last_error_ : "Database not open";
}

void Database::close() {
    if (conn_) {
        try {
            conn_->close();
        } catch (...) {
            // Ignore errors during close
        }
        conn_ = nullptr;
    }
}

// ===================
// Statement class
// ===================

Statement::Statement(Database& db, const std::string& sql)
    : db_(db), sql_(sql), current_row_(0), executed_(false), done_(false) {
    if (!db_.isOpen()) {
        work_ = nullptr;
        return;
    }

    try {
        work_ = std::make_unique<pqxx::work>(*db_.getHandle());
    } catch (const std::exception& e) {
        std::cerr << "Failed to create transaction: " << e.what() << std::endl;
        work_ = nullptr;
    }
}

Statement::~Statement() {
    if (work_) {
        try {
            // Commit the transaction if it's still active
            if (!executed_ || !done_) {
                work_->abort();
            } else {
                work_->commit();
            }
        } catch (...) {
            // Ignore errors during cleanup
        }
    }
}

bool Statement::bindInt(int index, int value) {
    if (!work_) return false;
    // Resize params vector if needed
    if (static_cast<size_t>(index) > params_.size()) {
        params_.resize(index);
    }
    params_[index - 1] = std::to_string(value);
    return true;
}

bool Statement::bindDouble(int index, double value) {
    if (!work_) return false;
    if (static_cast<size_t>(index) > params_.size()) {
        params_.resize(index);
    }
    params_[index - 1] = std::to_string(value);
    return true;
}

bool Statement::bindText(int index, const std::string& value) {
    if (!work_) return false;
    if (static_cast<size_t>(index) > params_.size()) {
        params_.resize(index);
    }
    params_[index - 1] = value;
    return true;
}

bool Statement::bindNull(int index) {
    if (!work_) return false;
    if (static_cast<size_t>(index) > params_.size()) {
        params_.resize(index);
    }
    // Use empty string to represent NULL, will be handled in exec_params
    params_[index - 1] = "";
    return true;
}

int Statement::step() {
    if (!work_) return SQLITE_ERROR;

    try {
        if (!executed_) {
            // Replace ? placeholders with $1, $2, etc. for PostgreSQL
            std::string pg_sql = sql_;
            int param_num = 1;
            size_t pos = 0;
            while ((pos = pg_sql.find('?', pos)) != std::string::npos) {
                pg_sql.replace(pos, 1, "$" + std::to_string(param_num++));
                pos += std::to_string(param_num - 1).length() + 1;
            }

            // Execute with parameters
            result_ = work_->exec_params(pg_sql, pqxx::prepare::make_dynamic_params(params_));

            // Check if this was an INSERT and try to get the last inserted ID
            if (pg_sql.find("INSERT") != std::string::npos ||
                pg_sql.find("insert") != std::string::npos) {
                // Try to extract the ID from RETURNING clause if present
                if (!result_.empty() && result_[0].size() > 0) {
                    try {
                        db_.last_insert_id_ = result_[0][0].as<long long>();
                    } catch (...) {
                        // If conversion fails, keep previous value
                    }
                }
            }

            executed_ = true;
            current_row_ = 0;
        }

        if (current_row_ < result_.size()) {
            current_row_++;
            return SQLITE_ROW;
        } else {
            if (!done_) {
                work_->commit();
                done_ = true;
            }
            return SQLITE_DONE;
        }
    } catch (const std::exception& e) {
        std::cerr << "Statement execution error: " << e.what() << std::endl;
        db_.last_error_ = e.what();
        return SQLITE_ERROR;
    }
}

bool Statement::reset() {
    if (!work_) return false;
    current_row_ = 0;
    executed_ = false;
    done_ = false;
    return true;
}

int Statement::getInt(int index) const {
    if (!executed_ || current_row_ == 0 || current_row_ > result_.size()) return 0;
    try {
        return result_[current_row_ - 1][index].as<int>();
    } catch (...) {
        return 0;
    }
}

long long Statement::getInt64(int index) const {
    if (!executed_ || current_row_ == 0 || current_row_ > result_.size()) return 0;
    try {
        return result_[current_row_ - 1][index].as<long long>();
    } catch (...) {
        return 0;
    }
}

double Statement::getDouble(int index) const {
    if (!executed_ || current_row_ == 0 || current_row_ > result_.size()) return 0.0;
    try {
        return result_[current_row_ - 1][index].as<double>();
    } catch (...) {
        return 0.0;
    }
}

std::string Statement::getText(int index) const {
    if (!executed_ || current_row_ == 0 || current_row_ > result_.size()) return "";
    try {
        return result_[current_row_ - 1][index].as<std::string>();
    } catch (...) {
        return "";
    }
}

bool Statement::isNull(int index) const {
    if (!executed_ || current_row_ == 0 || current_row_ > result_.size()) return true;
    try {
        return result_[current_row_ - 1][index].is_null();
    } catch (...) {
        return true;
    }
}

size_t Statement::affectedRows() const {
    if (!executed_) return 0;
    return result_.affected_rows();
}

} // namespace db
} // namespace sohbet
