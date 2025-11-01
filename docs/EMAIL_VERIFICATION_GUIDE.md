# Email Verification System Implementation Guide

**Status**: Backend Infrastructure Complete (50%)  
**Date**: October 28, 2025

---

## Overview

The email verification system ensures that users verify their email addresses during registration, enhancing security and reducing spam accounts.

## Implementation Status

### ✅ Completed (Backend Infrastructure)

1. **Database Schema** - Migration `002_email_verification.sql`
   - `email_verification_tokens` table created
   - `email_verified` column added to `users` table
   - Indexes created for performance

2. **Models** - `EmailVerificationToken`
   - Token generation and validation
   - Expiration checking (24 hours default)
   - JSON serialization

3. **Repository** - `EmailVerificationTokenRepository`
   - `createToken(user_id)` - Generate verification token
   - `findByToken(token)` - Retrieve token by string
   - `verifyToken(token)` - Mark token as verified and update user
   - `deleteExpiredTokens()` - Cleanup old tokens
   - `findLatestByUserId(user_id)` - Get user's latest token
   - `deleteTokensForUser(user_id)` - Remove all user tokens

4. **User Model Updates**
   - `email_verified` boolean field added
   - Included in JSON serialization
   - UserRepository updated to read/write the field

### ⚠️ Pending (API & Integration)

1. **Server Endpoints** - Add to `src/server/server.cpp`
   ```cpp
   // POST /api/email/send-verification
   // Request: {"user_id": 123} or from JWT token
   // Response: {"message": "Verification email sent", "token_id": 1}
   
   // GET /api/email/verify/:token
   // Response: {"message": "Email verified successfully", "user_id": 123}
   
   // POST /api/email/resend-verification
   // Request: {"email": "user@example.com"}
   // Response: {"message": "Verification email resent"}
   ```

2. **Email Service** - Create `src/services/email_service.cpp`
   - SMTP client implementation OR
   - Integration with SendGrid/Mailgun/AWS SES
   - Email template rendering
   - Send verification email with token link

3. **Integration Points**
   - Modify user registration endpoint to:
     * Create user
     * Generate verification token
     * Send verification email
     * Return success (without requiring immediate verification)
   - Add middleware to check `email_verified` status on protected routes

### 📝 Not Started (Frontend)

1. **React Components**
   - `EmailVerificationPrompt` - Show unverified banner
   - `EmailVerificationSuccess` - Show after successful verification
   - `ResendVerificationButton` - Allow users to request new email

2. **Pages/Routes**
   - `/verify-email/:token` - Verification landing page
   - Email verification status in user profile

3. **API Client** - Add to `frontend/app/lib/api-service.ts`
   ```typescript
   sendVerificationEmail(userId?: number): Promise<void>
   verifyEmail(token: string): Promise<User>
   resendVerificationEmail(email: string): Promise<void>
   ```

---

## Implementation Steps

### Step 1: Add Server Endpoints (C++)

**File**: `src/server/server.cpp`

Add repository initialization:
```cpp
#include "repositories/email_verification_token_repository.h"

// In constructor or initialization:
auto email_token_repo = std::make_shared<repositories::EmailVerificationTokenRepository>(database_);
```

Add endpoints:
```cpp
// Send verification email
server.Post("/api/email/send-verification", [email_token_repo, email_service](const httplib::Request& req, httplib::Response& res) {
    // Extract user_id from JWT or request body
    int user_id = extractUserIdFromJWT(req);
    
    // Generate token
    auto token = email_token_repo->createToken(user_id);
    if (!token) {
        res.status = 500;
        res.set_content("{\"error\":\"Failed to create token\"}", "application/json");
        return;
    }
    
    // Send email
    bool sent = email_service->sendVerificationEmail(user_email, token->getToken());
    if (sent) {
        res.set_content("{\"message\":\"Verification email sent\"}", "application/json");
    } else {
        res.status = 500;
        res.set_content("{\"error\":\"Failed to send email\"}", "application/json");
    }
});

// Verify email token
server.Get("/api/email/verify/:token", [email_token_repo](const httplib::Request& req, httplib::Response& res) {
    std::string token = req.path_params.at("token");
    
    bool verified = email_token_repo->verifyToken(token);
    if (verified) {
        res.set_content("{\"message\":\"Email verified successfully\"}", "application/json");
    } else {
        res.status = 400;
        res.set_content("{\"error\":\"Invalid or expired token\"}", "application/json");
    }
});
```

### Step 2: Implement Email Service

**Option A: SMTP (Using libcurl or custom socket)**

Create `include/services/email_service.h`:
```cpp
#pragma once
#include <string>

namespace sohbet {
namespace services {

class EmailService {
public:
    EmailService(const std::string& smtp_host, int smtp_port, 
                 const std::string& username, const std::string& password);
    
    bool sendVerificationEmail(const std::string& to_email, const std::string& token);
    bool sendEmail(const std::string& to, const std::string& subject, const std::string& body);
    
private:
    std::string smtp_host_;
    int smtp_port_;
    std::string username_;
    std::string password_;
    std::string from_email_;
    
    std::string renderVerificationEmail(const std::string& token);
};

} // namespace services
} // namespace sohbet
```

**Option B: Third-Party Service (Recommended)**

Use SendGrid, Mailgun, or AWS SES via their REST APIs:
```cpp
// Example using libcurl to call SendGrid API
bool EmailService::sendEmailViaAPI(const std::string& to, const std::string& subject, const std::string& html) {
    CURL* curl = curl_easy_init();
    struct curl_slist* headers = NULL;
    
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key_).c_str());
    
    std::string json = R"({
        "personalizations": [{"to": [{"email": ")" + to + R"("}]}],
        "from": {"email": ")" + from_email_ + R"("},
        "subject": ")" + subject + R"(",
        "content": [{"type": "text/html", "value": ")" + html + R"("}]
    })";
    
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.sendgrid.com/v3/mail/send");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    return res == CURLE_OK;
}
```

### Step 3: Email Template

Create an HTML email template:
```cpp
std::string EmailService::renderVerificationEmail(const std::string& token) {
    std::string verification_url = base_url_ + "/verify-email/" + token;
    
    return R"(
        <!DOCTYPE html>
        <html>
        <head>
            <style>
                body { font-family: Arial, sans-serif; line-height: 1.6; color: #333; }
                .container { max-width: 600px; margin: 0 auto; padding: 20px; }
                .button { 
                    display: inline-block; 
                    padding: 12px 24px; 
                    background-color: #4CAF50; 
                    color: white; 
                    text-decoration: none; 
                    border-radius: 4px; 
                    margin: 20px 0;
                }
            </style>
        </head>
        <body>
            <div class="container">
                <h2>Welcome to Sohbet!</h2>
                <p>Thank you for registering. Please verify your email address by clicking the button below:</p>
                <a href=")" + verification_url + R"(" class="button">Verify Email Address</a>
                <p>Or copy and paste this link into your browser:</p>
                <p>)" + verification_url + R"(</p>
                <p>This link will expire in 24 hours.</p>
                <p>If you didn't create an account, you can safely ignore this email.</p>
            </div>
        </body>
        </html>
    )";
}
```

### Step 4: Update Registration Flow

Modify `POST /api/users` endpoint:
```cpp
// After creating user successfully
if (user.has_value()) {
    // Generate verification token
    auto token = email_token_repo->createToken(user->getId().value());
    
    // Send verification email (don't fail registration if email fails)
    if (token.has_value()) {
        email_service->sendVerificationEmail(user->getEmail(), token->getToken());
    }
    
    // Return user data (email_verified will be false)
    res.set_content(user->toJson(), "application/json");
}
```

### Step 5: Frontend Implementation

**Verification Page** - `frontend/app/verify-email/[token]/page.tsx`:
```tsx
'use client';

import { use, useEffect, useState } from 'react';
import { useRouter } from 'next/navigation';
import { verifyEmail } from '@/lib/api-service';

export default function VerifyEmailPage({ params }: { params: Promise<{ token: string }> }) {
  const { token } = use(params);
  const router = useRouter();
  const [status, setStatus] = useState<'loading' | 'success' | 'error'>('loading');
  const [message, setMessage] = useState('');

  useEffect(() => {
    verifyEmail(token)
      .then(() => {
        setStatus('success');
        setMessage('Email verified successfully! Redirecting...');
        setTimeout(() => router.push('/dashboard'), 2000);
      })
      .catch((error) => {
        setStatus('error');
        setMessage(error.message || 'Failed to verify email');
      });
  }, [token, router]);

  return (
    <div className="container mx-auto p-8 text-center">
      {status === 'loading' && <p>Verifying your email...</p>}
      {status === 'success' && <p className="text-green-600">{message}</p>}
      {status === 'error' && <p className="text-red-600">{message}</p>}
    </div>
  );
}
```

**Unverified Banner** - `frontend/app/components/email-verification-banner.tsx`:
```tsx
'use client';

import { useState } from 'react';
import { useAuth } from '@/lib/auth-context';
import { resendVerificationEmail } from '@/lib/api-service';

export default function EmailVerificationBanner() {
  const { user } = useAuth();
  const [sent, setSent] = useState(false);

  if (!user || user.email_verified) return null;

  const handleResend = async () => {
    await resendVerificationEmail(user.email);
    setSent(true);
  };

  return (
    <div className="bg-yellow-100 border-l-4 border-yellow-500 p-4">
      <p className="text-yellow-700">
        Please verify your email address. Check your inbox for a verification link.
        {!sent && (
          <button onClick={handleResend} className="ml-2 underline">
            Resend verification email
          </button>
        )}
        {sent && <span className="ml-2 text-green-600">Verification email sent!</span>}
      </p>
    </div>
  );
}
```

---

## Environment Configuration

Add to `.env`:
```
# Email Service
SMTP_HOST=smtp.gmail.com
SMTP_PORT=587
SMTP_USER=your-email@gmail.com
SMTP_PASSWORD=your-app-password
SMTP_FROM=noreply@sohbet.app

# Or use SendGrid
SENDGRID_API_KEY=SG.xxx
SENDGRID_FROM_EMAIL=noreply@sohbet.app

# Verification URL
EMAIL_VERIFICATION_URL=https://sohbet.app/verify-email
```

---

## Testing

### Backend Tests

Create `tests/test_email_verification.cpp`:
```cpp
TEST_CASE("Email Verification Token Creation") {
    Database db(":memory:");
    // Run migrations
    EmailVerificationTokenRepository repo(db);
    
    auto token = repo.createToken(1);
    REQUIRE(token.has_value());
    REQUIRE(!token->getToken().empty());
    REQUIRE(token->getUserId() == 1);
}

TEST_CASE("Email Verification Token Validation") {
    // Test expired token
    // Test invalid token
    // Test successful verification
}
```

### Manual Testing

1. Register a new user
2. Check database for verification token
3. Test email sending (check spam folder)
4. Click verification link
5. Confirm `email_verified` = 1 in database
6. Try logging in and accessing protected routes

---

## Security Considerations

1. **Token Expiration**: Tokens expire after 24 hours
2. **One-Time Use**: Tokens can only be verified once
3. **Secure Transmission**: Send tokens via email, not in URL (in production, use HTTPS)
4. **Rate Limiting**: Limit resend requests to prevent abuse
5. **Token Complexity**: 64-character hex tokens are cryptographically secure

---

## Deployment Notes

### Fly.io (Backend)
- Configure SMTP credentials as secrets: `flyctl secrets set SMTP_PASSWORD=xxx`
- Or use SendGrid add-on

### Vercel (Frontend)
- Set `NEXT_PUBLIC_API_URL` to Fly.io backend URL
- Ensure verification redirect URL matches production domain

---

## Future Enhancements

- [ ] Email templates with branding
- [ ] Automatic token cleanup job (cron)
- [ ] Email verification reminder emails
- [ ] Account suspension after X days unverified
- [ ] Admin panel to manually verify users
- [ ] Multi-language email templates

---

**Next Priority**: After completing email verification, proceed with Rate Limiting implementation.
