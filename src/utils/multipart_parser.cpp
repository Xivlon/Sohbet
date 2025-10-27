#include "utils/multipart_parser.h"
#include <sstream>
#include <algorithm>

namespace sohbet {
namespace utils {

std::map<std::string, MultipartPart> MultipartParser::parse(
    const std::string& body,
    const std::string& boundary
) {
    std::map<std::string, MultipartPart> parts;
    
    if (body.empty() || boundary.empty()) {
        return parts;
    }
    
    std::string delimiter = "--" + boundary;
    std::string end_delimiter = "--" + boundary + "--";
    
    size_t pos = 0;
    while (pos < body.length()) {
        // Find next boundary
        size_t boundary_start = body.find(delimiter, pos);
        if (boundary_start == std::string::npos) {
            break;
        }
        
        // Skip the boundary line
        size_t content_start = body.find("\r\n", boundary_start);
        if (content_start == std::string::npos) {
            break;
        }
        content_start += 2; // Skip \r\n
        
        // Find next boundary
        size_t next_boundary = body.find(delimiter, content_start);
        if (next_boundary == std::string::npos) {
            break;
        }
        
        // Extract this part
        std::string part_content = body.substr(content_start, next_boundary - content_start);
        
        // Split headers and body
        size_t headers_end = part_content.find("\r\n\r\n");
        if (headers_end == std::string::npos) {
            pos = next_boundary;
            continue;
        }
        
        std::string headers_section = part_content.substr(0, headers_end);
        std::string body_section = part_content.substr(headers_end + 4);
        
        // Remove trailing \r\n from body
        if (body_section.length() >= 2 && 
            body_section.substr(body_section.length() - 2) == "\r\n") {
            body_section = body_section.substr(0, body_section.length() - 2);
        }
        
        // Parse headers
        auto headers = parseHeaders(headers_section);
        
        // Create part
        MultipartPart part;
        part.headers = headers;
        
        // Parse Content-Disposition
        if (headers.find("Content-Disposition") != headers.end()) {
            parseContentDisposition(headers["Content-Disposition"], part.name, part.filename);
        }
        
        // Get Content-Type
        if (headers.find("Content-Type") != headers.end()) {
            part.content_type = headers["Content-Type"];
        }
        
        // Store data
        part.data.assign(body_section.begin(), body_section.end());
        
        if (!part.name.empty()) {
            parts[part.name] = part;
        }
        
        pos = next_boundary;
    }
    
    return parts;
}

std::optional<std::string> MultipartParser::extractBoundary(const std::string& content_type) {
    std::string boundary_prefix = "boundary=";
    size_t pos = content_type.find(boundary_prefix);
    if (pos == std::string::npos) {
        return std::nullopt;
    }
    
    pos += boundary_prefix.length();
    size_t end_pos = content_type.find(';', pos);
    
    std::string boundary = (end_pos == std::string::npos) 
        ? content_type.substr(pos)
        : content_type.substr(pos, end_pos - pos);
    
    // Remove quotes if present
    if (!boundary.empty() && boundary.front() == '"' && boundary.back() == '"') {
        boundary = boundary.substr(1, boundary.length() - 2);
    }
    
    return trim(boundary);
}

void MultipartParser::parseContentDisposition(
    const std::string& header,
    std::string& name,
    std::string& filename
) {
    // Parse: Content-Disposition: form-data; name="field_name"; filename="file.jpg"
    
    size_t name_pos = header.find("name=\"");
    if (name_pos != std::string::npos) {
        name_pos += 6; // Skip 'name="'
        size_t name_end = header.find('"', name_pos);
        if (name_end != std::string::npos) {
            name = header.substr(name_pos, name_end - name_pos);
        }
    }
    
    size_t filename_pos = header.find("filename=\"");
    if (filename_pos != std::string::npos) {
        filename_pos += 10; // Skip 'filename="'
        size_t filename_end = header.find('"', filename_pos);
        if (filename_end != std::string::npos) {
            filename = header.substr(filename_pos, filename_end - filename_pos);
        }
    }
}

std::map<std::string, std::string> MultipartParser::parseHeaders(const std::string& headers_section) {
    std::map<std::string, std::string> headers;
    std::istringstream stream(headers_section);
    std::string line;
    
    while (std::getline(stream, line)) {
        // Remove \r if present
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        if (line.empty()) {
            continue;
        }
        
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string header_name = trim(line.substr(0, colon_pos));
            std::string header_value = trim(line.substr(colon_pos + 1));
            headers[header_name] = header_value;
        }
    }
    
    return headers;
}

std::string MultipartParser::trim(const std::string& str) {
    auto start = std::find_if_not(str.begin(), str.end(), ::isspace);
    auto end = std::find_if_not(str.rbegin(), str.rend(), ::isspace).base();
    
    return (start < end) ? std::string(start, end) : std::string();
}

} // namespace utils
} // namespace sohbet
