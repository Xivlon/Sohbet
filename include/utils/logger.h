#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <mutex>

namespace sohbet {
namespace utils {

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3
};

class Logger {
public:
    static Logger& getInstance();

    void setLevel(LogLevel level);
    LogLevel getLevel() const;

    void log(LogLevel level, const std::string& message, const std::string& context = "");

    void debug(const std::string& message, const std::string& context = "");
    void info(const std::string& message, const std::string& context = "");
    void warn(const std::string& message, const std::string& context = "");
    void error(const std::string& message, const std::string& context = "");

private:
    Logger();
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::string getCurrentTimestamp() const;
    std::string levelToString(LogLevel level) const;
    std::string escapeJson(const std::string& str) const;

    LogLevel min_level_;
    std::mutex mutex_;
};

// Convenience macros
#define LOG_DEBUG(msg) sohbet::utils::Logger::getInstance().debug(msg, __func__)
#define LOG_INFO(msg) sohbet::utils::Logger::getInstance().info(msg, __func__)
#define LOG_WARN(msg) sohbet::utils::Logger::getInstance().warn(msg, __func__)
#define LOG_ERROR(msg) sohbet::utils::Logger::getInstance().error(msg, __func__)

#define LOG_DEBUG_CTX(msg, ctx) sohbet::utils::Logger::getInstance().debug(msg, ctx)
#define LOG_INFO_CTX(msg, ctx) sohbet::utils::Logger::getInstance().info(msg, ctx)
#define LOG_WARN_CTX(msg, ctx) sohbet::utils::Logger::getInstance().warn(msg, ctx)
#define LOG_ERROR_CTX(msg, ctx) sohbet::utils::Logger::getInstance().error(msg, ctx)

} // namespace utils
} // namespace sohbet
