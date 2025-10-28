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
    
    // Test default WebSocket port
    unsetenv("WS_PORT");
    int default_ws_port = sohbet::config::get_websocket_port();
    assert(default_ws_port == 8081);
    std::cout << "✓ Default WebSocket port: " << default_ws_port << std::endl;
    
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
    
    // Restore original environment
    restore_env("PORT", orig_port);
    restore_env("WS_PORT", orig_ws_port);
    
    std::cout << "All environment configuration tests passed!" << std::endl;
    return 0;
}
