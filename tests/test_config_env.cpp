#include "config/env.h"
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <cstring>

// Helper to save environment variable
std::string save_env(const char* name) {
    const char* value = std::getenv(name);
    return value ? std::string(value) : "";
}

// Helper to restore environment variable
void restore_env(const char* name, const std::string& value) {
    if (value.empty()) {
        unsetenv(name);
    } else {
        setenv(name, value.c_str(), 1);
    }
}

int main() {
    std::cout << "Testing environment configuration..." << std::endl;
    
    // Save original environment
    std::string orig_port = save_env("PORT");
    std::string orig_ws_port = save_env("WS_PORT");
    std::string orig_cors = save_env("CORS_ORIGIN");
    
    // Test default HTTP port
    unsetenv("PORT");
    int default_http_port = sohbet::config::get_http_port();
    assert(default_http_port == 8080);
    std::cout << "✓ Default HTTP port: " << default_http_port << std::endl;
    
    // Test custom HTTP port
    setenv("PORT", "9090", 1);
    int custom_http_port = sohbet::config::get_http_port();
    assert(custom_http_port == 9090);
    std::cout << "✓ Custom HTTP port: " << custom_http_port << std::endl;
    
    // Test default WebSocket port (should match HTTP port)
    unsetenv("WS_PORT");
    unsetenv("PORT");
    int default_ws_port = sohbet::config::get_websocket_port();
    assert(default_ws_port == 8080);
    std::cout << "✓ Default WebSocket port (same as HTTP): " << default_ws_port << std::endl;
    
    // Test custom WebSocket port
    setenv("WS_PORT", "9091", 1);
    int custom_ws_port = sohbet::config::get_websocket_port();
    assert(custom_ws_port == 9091);
    std::cout << "✓ Custom WebSocket port: " << custom_ws_port << std::endl;
    
    // Test with string values
    setenv("PORT", "3000", 1);
    setenv("WS_PORT", "3001", 1);
    assert(sohbet::config::get_http_port() == 3000);
    assert(sohbet::config::get_websocket_port() == 3001);
    std::cout << "✓ Port configuration from environment variables works correctly" << std::endl;
    
    // Test default CORS origin (should be "http://localhost:5000")
    unsetenv("CORS_ORIGIN");
    std::string default_cors = sohbet::config::get_cors_origin();
    assert(default_cors == "http://localhost:5000");
    std::cout << "✓ Default CORS origin: " << default_cors << std::endl;
    
    // Test custom CORS origin
    setenv("CORS_ORIGIN", "https://example.com", 1);
    std::string custom_cors = sohbet::config::get_cors_origin();
    assert(custom_cors == "https://example.com");
    std::cout << "✓ Custom CORS origin: " << custom_cors << std::endl;
    
    // Test with production frontend URL
    setenv("CORS_ORIGIN", "https://sohbet-henna.vercel.app", 1);
    std::string production_cors = sohbet::config::get_cors_origin();
    assert(production_cors == "https://sohbet-henna.vercel.app");
    std::cout << "✓ Production CORS origin: " << production_cors << std::endl;
    
    // Restore original environment
    restore_env("PORT", orig_port);
    restore_env("WS_PORT", orig_ws_port);
    restore_env("CORS_ORIGIN", orig_cors);
    
    std::cout << "All environment configuration tests passed!" << std::endl;
    return 0;
}
