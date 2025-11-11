#include "server/server.h"
#include <iostream>
#include <cassert>
#include <cstdlib>

void test_demo_student_post_creation() {
    std::cout << "Testing demo_student post creation without JWT token..." << std::endl;
    
    // Set JWT secret for testing
    setenv("SOHBET_JWT_SECRET", "test_secret_key_for_unit_tests_only", 1);
    
    // Create server with in-memory database
    sohbet::server::AcademicSocialServer server(8080, ":memory:");
    assert(server.initialize());
    
    // Manually run the social features migration for in-memory database
    const std::string migration_sql = R"(
        CREATE TABLE IF NOT EXISTS roles (
            id SERIAL PRIMARY KEY,
            name TEXT UNIQUE NOT NULL,
            description TEXT,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );

        CREATE TABLE IF NOT EXISTS role_permissions (
            id SERIAL PRIMARY KEY,
            role_id INTEGER NOT NULL,
            permission TEXT NOT NULL,
            FOREIGN KEY (role_id) REFERENCES roles(id) ON DELETE CASCADE,
            UNIQUE(role_id, permission)
        );

        CREATE TABLE IF NOT EXISTS user_roles (
            id SERIAL PRIMARY KEY,
            user_id INTEGER NOT NULL,
            role_id INTEGER NOT NULL,
            assigned_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
            FOREIGN KEY (role_id) REFERENCES roles(id) ON DELETE CASCADE,
            UNIQUE(user_id, role_id)
        );

        CREATE TABLE IF NOT EXISTS posts (
            id SERIAL PRIMARY KEY,
            author_id INTEGER NOT NULL,
            author_type TEXT DEFAULT 'user',
            content TEXT NOT NULL,
            media_urls TEXT,
            visibility TEXT DEFAULT 'public',
            group_id INTEGER,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (author_id) REFERENCES users(id) ON DELETE CASCADE
        );

        INSERT INTO roles (name, description) VALUES
        ('Student', 'Default student account'),
        ('Professor', 'Faculty account'),
        ('Admin', 'Administrator account')
        ON CONFLICT (name) DO NOTHING;
    )";
    
    // Access the database through a handleRequest to trigger initialization
    sohbet::server::HttpRequest init_request("GET", "/api/status", "");
    server.handleRequest(init_request);
    
    // Test creating a post with demo_student username in body (no JWT token)
    std::string post_body = R"({
        "username": "demo_student",
        "content": "This is a test post from demo_student"
    })";
    
    sohbet::server::HttpRequest post_request("POST", "/api/posts", post_body);
    auto post_response = server.handleRequest(post_request);
    std::cout << "Post creation response: " << post_response.body << std::endl;
    std::cout << "Response status: " << post_response.status_code << std::endl;
    
    // Should succeed with 201 Created
    assert(post_response.status_code == 201);
    assert(post_response.body.find("\"content\":\"This is a test post from demo_student\"") != std::string::npos);
    assert(post_response.body.find("\"author_id\":") != std::string::npos);
    
    std::cout << "Demo student post creation test passed!" << std::endl;
}

void test_demo_student_post_creation_without_username() {
    std::cout << "Testing post creation without username (should fail)..." << std::endl;
    
    // Set JWT secret for testing
    setenv("SOHBET_JWT_SECRET", "test_secret_key_for_unit_tests_only", 1);
    
    // Create server with in-memory database
    sohbet::server::AcademicSocialServer server(8080, ":memory:");
    assert(server.initialize());
    
    // Test creating a post without username or auth token
    std::string post_body = R"({
        "content": "This post should fail"
    })";
    
    sohbet::server::HttpRequest post_request("POST", "/api/posts", post_body);
    auto post_response = server.handleRequest(post_request);
    std::cout << "Post creation response: " << post_response.body << std::endl;
    std::cout << "Response status: " << post_response.status_code << std::endl;
    
    // Should fail with 401 Unauthorized
    assert(post_response.status_code == 401);
    assert(post_response.body.find("\"error\":") != std::string::npos);
    
    std::cout << "Post creation without auth test passed!" << std::endl;
}

void test_non_demo_user_in_body() {
    std::cout << "Testing post creation with non-demo username in body (should fail)..." << std::endl;
    
    // Set JWT secret for testing
    setenv("SOHBET_JWT_SECRET", "test_secret_key_for_unit_tests_only", 1);
    
    // Create server with in-memory database
    sohbet::server::AcademicSocialServer server(8080, ":memory:");
    assert(server.initialize());
    
    // Test creating a post with a different username (not demo_student)
    std::string post_body = R"({
        "username": "other_user",
        "content": "This post should fail"
    })";
    
    sohbet::server::HttpRequest post_request("POST", "/api/posts", post_body);
    auto post_response = server.handleRequest(post_request);
    std::cout << "Post creation response: " << post_response.body << std::endl;
    std::cout << "Response status: " << post_response.status_code << std::endl;
    
    // Should fail with 401 Unauthorized
    assert(post_response.status_code == 401);
    assert(post_response.body.find("\"error\":") != std::string::npos);
    
    std::cout << "Non-demo user test passed!" << std::endl;
}

int main() {
    try {
        test_demo_student_post_creation();
        test_demo_student_post_creation_without_username();
        test_non_demo_user_in_body();
        std::cout << "All demo_student post creation tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
