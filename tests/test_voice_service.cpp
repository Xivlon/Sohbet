#include "voice/voice_service.h"
#include "voice/voice_config.h"
#include <iostream>
#include <cassert>

using namespace sohbet;

void test_voice_config() {
    std::cout << "Testing VoiceConfig..." << std::endl;
    
    VoiceConfig config;
    
    // Test default values
    assert(config.enabled == false);
    assert(config.murmur_host == "0.0.0.0");
    assert(config.murmur_port == 64738);
    assert(config.token_expiry_seconds == 300);
    assert(config.max_users_per_channel == 25);
    assert(config.enable_recording == false);
    
    // Test validation - disabled config should be valid
    assert(config.is_valid() == true);
    
    // Enable and test validation
    config.enabled = true;
    assert(config.is_valid() == true);
    
    // Test invalid port
    config.murmur_port = -1;
    assert(config.is_valid() == false);
    config.murmur_port = 64738;
    
    // Test invalid expiry
    config.token_expiry_seconds = 0;
    assert(config.is_valid() == false);
    config.token_expiry_seconds = 300;
    
    std::cout << "VoiceConfig tests passed!" << std::endl;
}

void test_voice_channel() {
    std::cout << "Testing VoiceChannel..." << std::endl;
    
    VoiceChannel channel;
    channel.id = 1;
    channel.name = "Test Channel";
    channel.channel_type = "public";
    channel.group_id = 0;
    channel.organization_id = 0;
    channel.created_at = std::time(nullptr);
    
    assert(channel.id == 1);
    assert(channel.name == "Test Channel");
    assert(channel.channel_type == "public");
    
    // Test JSON conversion
    std::string j = channel.to_json();
    assert(j.find("\"id\":1") != std::string::npos);
    assert(j.find("\"name\":\"Test Channel\"") != std::string::npos);
    assert(j.find("\"channel_type\":\"public\"") != std::string::npos);
    
    std::cout << "VoiceChannel tests passed!" << std::endl;
}

void test_voice_service_stub() {
    std::cout << "Testing VoiceServiceStub..." << std::endl;
    
    VoiceConfig config;
    config.enabled = true;
    
    VoiceServiceStub service(config);
    
    // Test service is enabled
    assert(service.is_enabled() == true);
    
    // Test creating a channel
    VoiceChannel channel = service.create_channel(
        "Study Group", 
        "public",
        0,
        0
    );
    
    assert(channel.id > 0);
    assert(channel.name == "Study Group");
    assert(channel.channel_type == "public");
    
    // Test listing channels
    auto channels = service.list_channels();
    assert(channels.size() == 1);
    assert(channels[0].id == channel.id);
    
    // Test getting a specific channel
    VoiceChannel retrieved = service.get_channel(channel.id);
    assert(retrieved.id == channel.id);
    assert(retrieved.name == channel.name);
    
    // Test generating connection token
    VoiceConnectionToken token = service.generate_connection_token(1, channel.id);
    assert(!token.token.empty());
    assert(token.murmur_host == config.murmur_host);
    assert(token.murmur_port == config.murmur_port);
    assert(token.expires_at > std::time(nullptr));
    
    // Test token validation
    assert(service.validate_token(token.token) == true);
    assert(service.validate_token("") == false);
    
    // Test deleting a channel
    assert(service.delete_channel(channel.id) == true);
    channels = service.list_channels();
    assert(channels.size() == 0);
    
    // Test deleting non-existent channel
    assert(service.delete_channel(999) == false);
    
    std::cout << "VoiceServiceStub tests passed!" << std::endl;
}

void test_voice_connection_token() {
    std::cout << "Testing VoiceConnectionToken..." << std::endl;
    
    VoiceConnectionToken token;
    token.token = "test_token_123";
    token.murmur_host = "voice.example.com";
    token.murmur_port = 64738;
    token.expires_at = std::time(nullptr) + 300;
    
    std::string j = token.to_json();
    assert(j.find("\"connection_token\":\"test_token_123\"") != std::string::npos);
    assert(j.find("\"murmur_host\":\"voice.example.com\"") != std::string::npos);
    assert(j.find("\"murmur_port\":64738") != std::string::npos);
    assert(j.find("\"expires_at\"") != std::string::npos);
    
    std::cout << "VoiceConnectionToken tests passed!" << std::endl;
}

int main() {
    try {
        test_voice_config();
        test_voice_channel();
        test_voice_connection_token();
        test_voice_service_stub();
        
        std::cout << "\n✓ All voice service tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
