#include "services/email_service.h"
#include "config/env.h"
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include <cstdlib>

namespace sohbet {
namespace services {

// Helper function for CURL write callback
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

EmailService::EmailService() {
    // Load configuration from environment variables
    const char* api_key = std::getenv("SENDGRID_API_KEY");
    if (api_key) {
        sendgrid_api_key_ = api_key;
    }

    const char* from_email = std::getenv("SENDGRID_FROM_EMAIL");
    if (from_email) {
        from_email_ = from_email;
    } else {
        from_email_ = "noreply@sohbet.app";
    }

    const char* verification_url = std::getenv("EMAIL_VERIFICATION_URL");
    if (verification_url) {
        verification_url_base_ = verification_url;
    } else {
        verification_url_base_ = "http://localhost:3000/verify-email";
    }

    const char* enabled = std::getenv("EMAIL_VERIFICATION_ENABLED");
    enabled_ = (enabled && std::string(enabled) == "true");

    if (!isConfigured() && enabled_) {
        std::cerr << "Warning: Email verification is enabled but SendGrid API key is not configured" << std::endl;
    }
}

bool EmailService::isConfigured() const {
    return !sendgrid_api_key_.empty();
}

bool EmailService::sendVerificationEmail(
    const std::string& to_email,
    const std::string& to_name,
    const std::string& verification_token
) {
    if (!enabled_) {
        std::cout << "Email verification is disabled. Skipping email send." << std::endl;
        return true; // Return success if disabled
    }

    if (!isConfigured()) {
        std::cerr << "Error: SendGrid API key not configured" << std::endl;
        return false;
    }

    std::string verification_url = verification_url_base_;
    if (verification_url.find('?') == std::string::npos) {
        verification_url += "?token=" + verification_token;
    } else {
        verification_url += "&token=" + verification_token;
    }

    std::string subject = "Verify your Sohbet account";
    std::string html_content = generateVerificationEmailHtml(to_name, verification_url);
    std::string text_content = generateVerificationEmailText(to_name, verification_url);

    return sendEmail(to_email, to_name, subject, html_content, text_content);
}

bool EmailService::sendPasswordResetEmail(
    const std::string& to_email,
    const std::string& to_name,
    const std::string& reset_token
) {
    if (!isConfigured()) {
        std::cerr << "Error: SendGrid API key not configured" << std::endl;
        return false;
    }

    // TODO: Implement password reset email
    std::cout << "Password reset email not yet implemented" << std::endl;
    return false;
}

bool EmailService::sendEmail(
    const std::string& to_email,
    const std::string& to_name,
    const std::string& subject,
    const std::string& html_content,
    const std::string& text_content
) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL" << std::endl;
        return false;
    }

    // Build JSON payload
    std::stringstream json_payload;
    json_payload << "{"
                 << "\"personalizations\":[{"
                 << "\"to\":[{\"email\":\"" << to_email << "\"";

    if (!to_name.empty()) {
        json_payload << ",\"name\":\"" << to_name << "\"";
    }

    json_payload << "}]"
                 << "}],"
                 << "\"from\":{\"email\":\"" << from_email_ << "\"},"
                 << "\"subject\":\"" << subject << "\","
                 << "\"content\":[";

    // Add text content
    if (!text_content.empty()) {
        // Escape text content for JSON
        std::string escaped_text = text_content;
        size_t pos = 0;
        while ((pos = escaped_text.find("\"", pos)) != std::string::npos) {
            escaped_text.replace(pos, 1, "\\\"");
            pos += 2;
        }
        pos = 0;
        while ((pos = escaped_text.find("\n", pos)) != std::string::npos) {
            escaped_text.replace(pos, 1, "\\n");
            pos += 2;
        }
        json_payload << "{\"type\":\"text/plain\",\"value\":\"" << escaped_text << "\"}";
    }

    // Add HTML content
    if (!html_content.empty()) {
        if (!text_content.empty()) {
            json_payload << ",";
        }
        // Escape HTML content for JSON
        std::string escaped_html = html_content;
        size_t pos = 0;
        while ((pos = escaped_html.find("\"", pos)) != std::string::npos) {
            escaped_html.replace(pos, 1, "\\\"");
            pos += 2;
        }
        pos = 0;
        while ((pos = escaped_html.find("\n", pos)) != std::string::npos) {
            escaped_html.replace(pos, 1, "\\n");
            pos += 2;
        }
        json_payload << "{\"type\":\"text/html\",\"value\":\"" << escaped_html << "\"}";
    }

    json_payload << "]"
                 << "}";

    std::string payload_str = json_payload.str();
    std::string response_string;
    std::string auth_header = "Authorization: Bearer " + sendgrid_api_key_;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, auth_header.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, "https://api.sendgrid.com/v3/mail/send");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload_str.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
        return false;
    }

    if (http_code >= 200 && http_code < 300) {
        std::cout << "Email sent successfully to " << to_email << std::endl;
        return true;
    } else {
        std::cerr << "SendGrid API error (HTTP " << http_code << "): " << response_string << std::endl;
        return false;
    }
}

std::string EmailService::generateVerificationEmailHtml(
    const std::string& name,
    const std::string& verification_url
) const {
    std::stringstream html;
    html << "<!DOCTYPE html>"
         << "<html>"
         << "<head><meta charset=\"UTF-8\"></head>"
         << "<body style=\"font-family: Arial, sans-serif; line-height: 1.6; color: #333;\">"
         << "<div style=\"max-width: 600px; margin: 0 auto; padding: 20px;\">"
         << "<h1 style=\"color: #4A5568;\">Welcome to Sohbet!</h1>";

    if (!name.empty()) {
        html << "<p>Hi " << name << ",</p>";
    } else {
        html << "<p>Hi there,</p>";
    }

    html << "<p>Thank you for creating an account with Sohbet, the academic social platform for students and educators.</p>"
         << "<p>To complete your registration and verify your email address, please click the button below:</p>"
         << "<div style=\"text-align: center; margin: 30px 0;\">"
         << "<a href=\"" << verification_url << "\" "
         << "style=\"background-color: #4299E1; color: white; padding: 12px 30px; "
         << "text-decoration: none; border-radius: 5px; display: inline-block;\">"
         << "Verify Email Address</a>"
         << "</div>"
         << "<p>Or copy and paste this link into your browser:</p>"
         << "<p style=\"word-break: break-all; color: #4299E1;\">" << verification_url << "</p>"
         << "<p style=\"margin-top: 30px; color: #718096; font-size: 14px;\">"
         << "If you didn't create an account with Sohbet, you can safely ignore this email.</p>"
         << "<hr style=\"border: none; border-top: 1px solid #E2E8F0; margin: 30px 0;\">"
         << "<p style=\"color: #A0AEC0; font-size: 12px;\">© 2024 Sohbet. All rights reserved.</p>"
         << "</div>"
         << "</body>"
         << "</html>";

    return html.str();
}

std::string EmailService::generateVerificationEmailText(
    const std::string& name,
    const std::string& verification_url
) const {
    std::stringstream text;
    text << "Welcome to Sohbet!\n\n";

    if (!name.empty()) {
        text << "Hi " << name << ",\n\n";
    } else {
        text << "Hi there,\n\n";
    }

    text << "Thank you for creating an account with Sohbet, the academic social platform for students and educators.\n\n"
         << "To complete your registration and verify your email address, please visit the following link:\n\n"
         << verification_url << "\n\n"
         << "If you didn't create an account with Sohbet, you can safely ignore this email.\n\n"
         << "Best regards,\n"
         << "The Sohbet Team\n\n"
         << "---\n"
         << "© 2024 Sohbet. All rights reserved.";

    return text.str();
}

} // namespace services
} // namespace sohbet
