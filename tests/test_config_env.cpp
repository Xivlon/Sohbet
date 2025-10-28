#include "config/env.h"
#include <iostream>
#include <cstdlib>
#include <cassert>

int main() {
    std::cout << "Testing environment configuration..." << std::endl;
    
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
    
    std::cout << "All environment configuration tests passed!" << std::endl;
    return 0;
}
