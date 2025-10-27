#include "utils/multipart_parser.h"
#include <iostream>
#include <cassert>
#include <string>

using namespace sohbet::utils;

void testMultipartParser() {
    std::cout << "Testing Multipart Parser..." << std::endl;
    
    // Test 1: Extract boundary
    std::cout << "Test 1: Extract boundary... ";
    std::string content_type1 = "multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW";
    auto boundary1 = MultipartParser::extractBoundary(content_type1);
    assert(boundary1.has_value());
    assert(boundary1.value() == "----WebKitFormBoundary7MA4YWxkTrZu0gW");
    
    std::string content_type2 = "multipart/form-data; boundary=\"----WebKitFormBoundary7MA4YWxkTrZu0gW\"";
    auto boundary2 = MultipartParser::extractBoundary(content_type2);
    assert(boundary2.has_value());
    assert(boundary2.value() == "----WebKitFormBoundary7MA4YWxkTrZu0gW");
    
    std::string content_type3 = "application/json";
    auto boundary3 = MultipartParser::extractBoundary(content_type3);
    assert(!boundary3.has_value());
    std::cout << "PASSED" << std::endl;
    
    // Test 2: Parse simple multipart data
    std::cout << "Test 2: Parse simple multipart data... ";
    std::string boundary = "----WebKitFormBoundary";
    std::string body = 
        "------WebKitFormBoundary\r\n"
        "Content-Disposition: form-data; name=\"field1\"\r\n"
        "\r\n"
        "value1\r\n"
        "------WebKitFormBoundary\r\n"
        "Content-Disposition: form-data; name=\"field2\"\r\n"
        "\r\n"
        "value2\r\n"
        "------WebKitFormBoundary--\r\n";
    
    auto parts = MultipartParser::parse(body, boundary);
    assert(parts.size() == 2);
    assert(parts.find("field1") != parts.end());
    assert(parts.find("field2") != parts.end());
    
    std::string field1_value(parts["field1"].data.begin(), parts["field1"].data.end());
    std::string field2_value(parts["field2"].data.begin(), parts["field2"].data.end());
    assert(field1_value == "value1");
    assert(field2_value == "value2");
    std::cout << "PASSED" << std::endl;
    
    // Test 3: Parse file upload
    std::cout << "Test 3: Parse file upload... ";
    std::string file_body = 
        "------WebKitFormBoundary\r\n"
        "Content-Disposition: form-data; name=\"user_id\"\r\n"
        "\r\n"
        "123\r\n"
        "------WebKitFormBoundary\r\n"
        "Content-Disposition: form-data; name=\"file\"; filename=\"test.jpg\"\r\n"
        "Content-Type: image/jpeg\r\n"
        "\r\n"
        "binary file data here\r\n"
        "------WebKitFormBoundary--\r\n";
    
    auto file_parts = MultipartParser::parse(file_body, boundary);
    assert(file_parts.size() == 2);
    assert(file_parts.find("user_id") != file_parts.end());
    assert(file_parts.find("file") != file_parts.end());
    
    auto& file_part = file_parts["file"];
    assert(file_part.filename == "test.jpg");
    assert(file_part.content_type == "image/jpeg");
    
    std::string file_data(file_part.data.begin(), file_part.data.end());
    assert(file_data == "binary file data here");
    std::cout << "PASSED" << std::endl;
    
    // Test 4: Parse multiple headers
    std::cout << "Test 4: Parse multiple headers... ";
    std::string multi_header_body = 
        "------WebKitFormBoundary\r\n"
        "Content-Disposition: form-data; name=\"photo\"; filename=\"photo.png\"\r\n"
        "Content-Type: image/png\r\n"
        "Content-Transfer-Encoding: binary\r\n"
        "\r\n"
        "PNG data\r\n"
        "------WebKitFormBoundary--\r\n";
    
    auto multi_parts = MultipartParser::parse(multi_header_body, boundary);
    assert(multi_parts.size() == 1);
    assert(multi_parts.find("photo") != multi_parts.end());
    
    auto& photo_part = multi_parts["photo"];
    assert(photo_part.filename == "photo.png");
    assert(photo_part.content_type == "image/png");
    assert(photo_part.headers.find("Content-Transfer-Encoding") != photo_part.headers.end());
    std::cout << "PASSED" << std::endl;
    
    // Test 5: Empty body
    std::cout << "Test 5: Empty body... ";
    auto empty_parts = MultipartParser::parse("", boundary);
    assert(empty_parts.size() == 0);
    std::cout << "PASSED" << std::endl;
    
    std::cout << "All Multipart Parser tests PASSED!" << std::endl;
}

int main() {
    try {
        testMultipartParser();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
