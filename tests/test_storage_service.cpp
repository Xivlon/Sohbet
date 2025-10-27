#include "services/storage_service.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <filesystem>

using namespace sohbet::services;

void testStorageService() {
    std::cout << "Testing Storage Service..." << std::endl;
    
    // Use a test directory
    std::string test_path = "/tmp/test_uploads/";
    StorageService storage(test_path);
    
    // Test 1: File type validation
    std::cout << "Test 1: File type validation... ";
    std::vector<std::string> allowed_types = {"image/jpeg", "image/png"};
    assert(StorageService::validateFileType("image/jpeg", allowed_types) == true);
    assert(StorageService::validateFileType("image/png", allowed_types) == true);
    assert(StorageService::validateFileType("image/gif", allowed_types) == false);
    assert(StorageService::validateFileType("application/pdf", allowed_types) == false);
    std::cout << "PASSED" << std::endl;
    
    // Test 2: File size validation
    std::cout << "Test 2: File size validation... ";
    assert(StorageService::validateFileSize(1024, 2048) == true);
    assert(StorageService::validateFileSize(2048, 2048) == true);
    assert(StorageService::validateFileSize(2049, 2048) == false);
    std::cout << "PASSED" << std::endl;
    
    // Test 3: Storage key generation
    std::cout << "Test 3: Storage key generation... ";
    std::string key1 = StorageService::generateStorageKey(1, "avatar", "test.jpg");
    std::string key2 = StorageService::generateStorageKey(1, "avatar", "test.jpg");
    
    // Keys should be unique
    assert(key1 != key2);
    
    // Keys should contain expected components
    assert(key1.find("user_1") != std::string::npos);
    assert(key1.find("avatar") != std::string::npos);
    assert(key1.find(".jpg") != std::string::npos);
    std::cout << "PASSED" << std::endl;
    
    // Test 4: Store and retrieve file
    std::cout << "Test 4: Store and retrieve file... ";
    std::vector<uint8_t> test_data = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; // "Hello"
    
    auto metadata = storage.storeFile(test_data, "test.txt", "text/plain", 1, "test");
    assert(metadata.has_value());
    assert(metadata->file_name == "test.txt");
    assert(metadata->mime_type == "text/plain");
    assert(metadata->file_size == 5);
    
    // Retrieve the file
    auto retrieved_data = storage.retrieveFile(metadata->storage_key);
    assert(retrieved_data.has_value());
    assert(retrieved_data->size() == test_data.size());
    assert(*retrieved_data == test_data);
    std::cout << "PASSED" << std::endl;
    
    // Test 5: File exists check
    std::cout << "Test 5: File exists check... ";
    assert(storage.fileExists(metadata->storage_key) == true);
    assert(storage.fileExists("nonexistent_file.txt") == false);
    std::cout << "PASSED" << std::endl;
    
    // Test 6: Delete file
    std::cout << "Test 6: Delete file... ";
    assert(storage.deleteFile(metadata->storage_key) == true);
    assert(storage.fileExists(metadata->storage_key) == false);
    assert(storage.deleteFile(metadata->storage_key) == false); // Already deleted
    std::cout << "PASSED" << std::endl;
    
    // Test 7: Empty file handling
    std::cout << "Test 7: Empty file handling... ";
    std::vector<uint8_t> empty_data;
    auto empty_metadata = storage.storeFile(empty_data, "empty.txt", "text/plain", 1, "test");
    assert(!empty_metadata.has_value()); // Should fail for empty data
    std::cout << "PASSED" << std::endl;
    
    // Cleanup test directory
    std::filesystem::remove_all(test_path);
    
    std::cout << "All Storage Service tests PASSED!" << std::endl;
}

int main() {
    try {
        testStorageService();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
