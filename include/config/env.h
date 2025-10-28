#pragma once
#include <string>
#include <cstdlib>
#include <stdexcept>

namespace sohbet {
namespace config {

inline std::string get_jwt_secret() {
    const char* secret = std::getenv("SOHBET_JWT_SECRET");
    if (!secret || std::string(secret).empty()) {
        const char* fallback = std::getenv("JWT_SECRET");
        if (!fallback || std::string(fallback).empty()) {
            throw std::runtime_error(
                "CRITICAL: JWT secret not configured. "
                "Set SOHBET_JWT_SECRET environment variable before starting the server."
            );
        }
        return std::string(fallback);
    }
    return std::string(secret);
}

inline int get_jwt_expiry_hours() {
    const char* expiry = std::getenv("JWT_EXPIRY_HOURS");
    if (!expiry) {
        return 24;
    }
    return std::atoi(expiry);
}

inline int get_http_port() {
    const char* port = std::getenv("PORT");
    if (!port) {
        return 8080;
    }
    return std::atoi(port);
}

inline int get_websocket_port() {
    const char* port = std::getenv("WS_PORT");
    if (!port) {
        return 8081;
    }
    return std::atoi(port);
}

inline std::string get_cors_origin() {
    const char* origin = std::getenv("CORS_ORIGIN");
    if (!origin || std::string(origin).empty()) {
        // Default to allowing all origins for development
        return "*";
    }
    return std::string(origin);
}

} // namespace config
} // namespace sohbet
