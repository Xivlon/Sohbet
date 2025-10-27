#pragma once

#include <string>
#include <map>
#include <vector>
#include <optional>
#include <cstdint>

namespace sohbet {
namespace utils {

/**
 * Represents a part in a multipart/form-data request
 */
struct MultipartPart {
    std::string name;
    std::string filename;
    std::string content_type;
    std::vector<uint8_t> data;
    std::map<std::string, std::string> headers;
};

/**
 * Multipart form data parser
 */
class MultipartParser {
public:
    /**
     * Parse multipart/form-data
     * @param body Request body
     * @param boundary Multipart boundary string
     * @return Map of field names to parts
     */
    static std::map<std::string, MultipartPart> parse(
        const std::string& body,
        const std::string& boundary
    );
    
    /**
     * Extract boundary from Content-Type header
     * @param content_type Content-Type header value
     * @return Boundary string if found, std::nullopt otherwise
     */
    static std::optional<std::string> extractBoundary(const std::string& content_type);
    
private:
    /**
     * Parse Content-Disposition header
     * @param header Content-Disposition header value
     * @param name Output parameter for field name
     * @param filename Output parameter for filename (if present)
     */
    static void parseContentDisposition(
        const std::string& header,
        std::string& name,
        std::string& filename
    );
    
    /**
     * Parse headers from a multipart section
     * @param headers_section Raw headers section
     * @return Map of header name to value
     */
    static std::map<std::string, std::string> parseHeaders(const std::string& headers_section);
    
    /**
     * Trim whitespace from string
     * @param str String to trim
     * @return Trimmed string
     */
    static std::string trim(const std::string& str);
};

} // namespace utils
} // namespace sohbet
