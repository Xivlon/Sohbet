#include "server/websocket_server.h"
#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>

using namespace sohbet::server;

void test_websocket_initialization() {
    std::cout << "Testing WebSocket initialization..." << std::endl;
    
    WebSocketServer server(8082);
    
    // Test that handlers can be registered
    bool handler_called = false;
    server.registerHandler("test:message", 
        [&handler_called](int user_id, const WebSocketMessage& msg) {
            handler_called = true;
        });
    
    std::cout << "✓ WebSocket initialization test passed" << std::endl;
}

void test_websocket_message_creation() {
    std::cout << "Testing WebSocket message creation..." << std::endl;
    
    WebSocketMessage msg("chat:send", "{\"content\":\"Hello\"}");
    assert(msg.type == "chat:send");
    assert(msg.payload == "{\"content\":\"Hello\"}");
    
    std::cout << "✓ WebSocket message creation test passed" << std::endl;
}

void test_websocket_connection() {
    std::cout << "Testing WebSocket connection..." << std::endl;
    
    // Create a connection object
    WebSocketConnection conn(123, 1);
    assert(conn.getSocketFd() == 123);
    assert(conn.getUserId() == 1);
    assert(conn.isAuthenticated() == true);
    
    std::cout << "✓ WebSocket connection test passed" << std::endl;
}

int main() {
    std::cout << "Running WebSocket Server Tests..." << std::endl;
    std::cout << "=================================" << std::endl;
    
    try {
        test_websocket_initialization();
        test_websocket_message_creation();
        test_websocket_connection();
        
        std::cout << "=================================" << std::endl;
        std::cout << "All WebSocket tests passed! ✓" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
