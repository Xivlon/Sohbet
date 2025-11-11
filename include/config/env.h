#pragma once
#include <string>
#include <cstdlib>
#include <stdexcept>
#include <iostream>

namespace sohbet {
namespace config {

inline std::string get_jwt_secret() {
    const char* secret = std::getenv("SOHBET_JWT_SECRET");
    if (!secret || std::string(secret).empty()) {
        const char* fallback = std::getenv("JWT_SECRET");
        if (!fallback || std::string(fallback).empty()) {
            // Development default - INSECURE, for demo/development only
            // This allows demo login to work out of the box
            std::cerr << "WARNING: Using default JWT secret. This is INSECURE and should only be used for development/demo.\n";
            std::cerr << "Set SOHBET_JWT_SECRET environment variable for production use.\n";
            return "dev-only-insecure-jwt-secret-change-in-production";
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
        // Default to localhost for development
        // Note: When using Access-Control-Allow-Credentials, origin cannot be "*"
        return "http://localhost:5000";
    }
    return std::string(origin);
}

inline std::string get_database_url() {
    const char* url = std::getenv("DATABASE_URL");
    if (!url || std::string(url).empty()) {
        std::cerr << "ERROR: DATABASE_URL environment variable is required for PostgreSQL connection.\n";
        std::cerr << "Please set DATABASE_URL to your PostgreSQL connection string.\n";
        std::cerr << "Example: postgresql://user:password@host:port/database\n";
        throw std::runtime_error("DATABASE_URL environment variable is required");
    }
    return std::string(url);
}

} // namespace config
} // namespace sohbet
