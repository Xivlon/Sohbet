#pragma once

#include <string>
#include <memory>

namespace sohbet {
namespace services {

/**
 * EmailService - handles sending emails using SendGrid API
 */
class EmailService {
public:
    EmailService();

    /**
     * Send a verification email to the user
     * @param to_email Recipient email address
     * @param to_name Recipient name (optional)
     * @param verification_token Token for email verification
     * @return true if email was sent successfully, false otherwise
     */
    bool sendVerificationEmail(
        const std::string& to_email,
        const std::string& to_name,
        const std::string& verification_token
    );

    /**
     * Send a password reset email
     * @param to_email Recipient email address
     * @param to_name Recipient name (optional)
     * @param reset_token Token for password reset
     * @return true if email was sent successfully, false otherwise
     */
    bool sendPasswordResetEmail(
        const std::string& to_email,
        const std::string& to_name,
        const std::string& reset_token
    );

    /**
     * Check if email service is configured
     * @return true if SendGrid API key is set, false otherwise
     */
    bool isConfigured() const;

private:
    std::string sendgrid_api_key_;
    std::string from_email_;
    std::string verification_url_base_;
    bool enabled_;

    /**
     * Send email via SendGrid API
     * @param to_email Recipient email
     * @param to_name Recipient name
     * @param subject Email subject
     * @param html_content HTML content
     * @param text_content Plain text content
     * @return true if sent successfully
     */
    bool sendEmail(
        const std::string& to_email,
        const std::string& to_name,
        const std::string& subject,
        const std::string& html_content,
        const std::string& text_content
    );

    /**
     * Generate HTML template for verification email
     */
    std::string generateVerificationEmailHtml(
        const std::string& name,
        const std::string& verification_url
    ) const;

    /**
     * Generate plain text template for verification email
     */
    std::string generateVerificationEmailText(
        const std::string& name,
        const std::string& verification_url
    ) const;
};

} // namespace services
} // namespace sohbet
