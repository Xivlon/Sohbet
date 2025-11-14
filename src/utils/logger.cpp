#include "utils/logger.h"
#include <cstdlib>
#include <ctime>

namespace sohbet {
namespace utils {

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() : min_level_(LogLevel::INFO) {
    // Check environment variable for log level
    const char* env_level = std::getenv("LOG_LEVEL");
    if (env_level) {
        std::string level_str(env_level);
        if (level_str == "DEBUG") {
            min_level_ = LogLevel::DEBUG;
        } else if (level_str == "INFO") {
            min_level_ = LogLevel::INFO;
        } else if (level_str == "WARN") {
            min_level_ = LogLevel::WARN;
        } else if (level_str == "ERROR") {
            min_level_ = LogLevel::ERROR;
        }
    }
}

void Logger::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    min_level_ = level;
}

LogLevel Logger::getLevel() const {
    return min_level_;
}

std::string Logger::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::tm tm_buf;
    gmtime_r(&time_t_now, &tm_buf);

    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count() << 'Z';
    return oss.str();
}

std::string Logger::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

std::string Logger::escapeJson(const std::string& str) const {
    std::ostringstream oss;
    for (char c : str) {
        switch (c) {
            case '"':  oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\b': oss << "\\b"; break;
            case '\f': oss << "\\f"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default:
                if (c < 0x20) {
                    oss << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                        << static_cast<int>(c);
                } else {
                    oss << c;
                }
                break;
        }
    }
    return oss.str();
}

void Logger::log(LogLevel level, const std::string& message, const std::string& context) {
    if (level < min_level_) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    // Output as structured JSON for Grafana/Loki
    std::cout << "{"
              << "\"timestamp\":\"" << getCurrentTimestamp() << "\","
              << "\"level\":\"" << levelToString(level) << "\","
              << "\"message\":\"" << escapeJson(message) << "\"";

    if (!context.empty()) {
        std::cout << ",\"context\":\"" << escapeJson(context) << "\"";
    }

    std::cout << "}" << std::endl;
}

void Logger::debug(const std::string& message, const std::string& context) {
    log(LogLevel::DEBUG, message, context);
}

void Logger::info(const std::string& message, const std::string& context) {
    log(LogLevel::INFO, message, context);
}

void Logger::warn(const std::string& message, const std::string& context) {
    log(LogLevel::WARN, message, context);
}

void Logger::error(const std::string& message, const std::string& context) {
    log(LogLevel::ERROR, message, context);
}

} // namespace utils
} // namespace sohbet
