# Logging and Monitoring Setup

This document describes the logging and monitoring setup for the Sohbet application.

## Structured Logging

The application uses a custom structured logger that outputs JSON-formatted logs, making them easy to parse and analyze with tools like Grafana Loki.

### Log Levels

The logger supports four log levels:
- `DEBUG`: Detailed diagnostic information
- `INFO`: General informational messages
- `WARN`: Warning messages for potentially harmful situations
- `ERROR`: Error messages for error events

### Configuration

Set the `LOG_LEVEL` environment variable to control the minimum log level:

```bash
export LOG_LEVEL=INFO  # Options: DEBUG, INFO, WARN, ERROR
```

### Usage in Code

```cpp
#include "utils/logger.h"

// Simple logging
LOG_INFO("Server started");
LOG_ERROR("Failed to connect to database");

// Logging with context
LOG_DEBUG_CTX("Processing request", "handleRequest");
LOG_WARN_CTX("Rate limit exceeded", "API");
```

### Log Format

Logs are output as JSON for easy parsing:

```json
{
  "timestamp": "2025-11-14T23:20:31.711Z",
  "level": "INFO",
  "message": "Server started successfully",
  "context": "main"
}
```

## Grafana Integration

### Searching Logs in Grafana with Loki

Fly.io automatically ships logs to Grafana Loki. To search logs:

1. Visit your Fly.io dashboard: https://fly.io/dashboard
2. Navigate to your app: `sohbet-uezxqq`
3. Click on "Monitoring" → "Logs"
4. Use LogQL to search logs:

```logql
# All error logs
{app="sohbet-uezxqq"} | json | level="ERROR"

# Logs from specific context
{app="sohbet-uezxqq"} | json | context="handleRequest"

# Logs containing specific text
{app="sohbet-uezxqq"} | json | message =~ ".*database.*"

# Rate of errors over time
rate({app="sohbet-uezxqq"} | json | level="ERROR" [5m])
```

### Accessing Grafana

```bash
# Access Grafana for your app
fly dashboard -a sohbet-uezxqq

# View logs directly from CLI
fly logs -a sohbet-uezxqq

# Follow logs in real-time
fly logs -a sohbet-uezxqq -f
```

## Sentry Integration

### Setup

1. Create a Sentry account at https://sentry.io
2. Create a new project for your application
3. Get your DSN (Data Source Name) from the project settings
4. Add the Sentry DSN to your Fly.io secrets:

```bash
fly secrets set SENTRY_DSN="https://your-sentry-dsn@sentry.io/project-id" -a sohbet-uezxqq
```

### Installing Sentry SDK

To integrate Sentry natively in C++, add the sentry-native SDK to CMakeLists.txt:

```cmake
# Add to CMakeLists.txt
FetchContent_Declare(
    sentry
    GIT_REPOSITORY https://github.com/getsentry/sentry-native.git
    GIT_TAG 0.7.0
)
set(SENTRY_BUILD_EXAMPLES OFF)
set(SENTRY_BUILD_TESTS OFF)
FetchContent_MakeAvailable(sentry)

# Link to your executable
target_link_libraries(sohbet_lib sentry)
```

### Sentry Integration Code

Create a Sentry utility wrapper:

```cpp
// include/utils/sentry_util.h
#pragma once
#include <string>

namespace sohbet {
namespace utils {

class SentryUtil {
public:
    static void initialize(const std::string& dsn);
    static void captureError(const std::string& message, const std::string& level = "error");
    static void captureException(const std::exception& e);
    static void addBreadcrumb(const std::string& message, const std::string& category = "default");
    static void shutdown();
};

} // namespace utils
} // namespace sohbet
```

### Usage Example

```cpp
#include "utils/sentry_util.h"

int main() {
    // Initialize Sentry
    const char* sentry_dsn = std::getenv("SENTRY_DSN");
    if (sentry_dsn) {
        SentryUtil::initialize(sentry_dsn);
    }

    try {
        // Your code here
    } catch (const std::exception& e) {
        LOG_ERROR(e.what());
        SentryUtil::captureException(e);
    }

    SentryUtil::shutdown();
    return 0;
}
```

## Environment Variables

Add these to your `.env` or Fly.io secrets:

```bash
# Logging
LOG_LEVEL=INFO

# Sentry (optional)
SENTRY_DSN=https://your-sentry-dsn@sentry.io/project-id
SENTRY_ENVIRONMENT=production
SENTRY_RELEASE=0.3.0-academic
```

## Monitoring Best Practices

1. **Use appropriate log levels**: Don't log everything at ERROR level
2. **Add context**: Always provide context with your logs
3. **Structured data**: Use JSON format for easy parsing
4. **Error tracking**: Capture exceptions and errors in Sentry
5. **Performance monitoring**: Use Sentry's performance monitoring features
6. **Alerts**: Set up alerts in Grafana for critical errors

## Troubleshooting

### Logs not appearing in Grafana

1. Check that your app is running: `fly status -a sohbet-uezxqq`
2. Verify logs are being produced: `fly logs -a sohbet-uezxqq`
3. Wait a few minutes for logs to appear in Grafana (there can be a delay)

### Sentry not capturing errors

1. Verify `SENTRY_DSN` is set: `fly secrets list -a sohbet-uezxqq`
2. Check that Sentry SDK is properly initialized
3. Look for Sentry initialization errors in logs
4. Test with a manual error capture

## References

- [Fly.io Logging Documentation](https://fly.io/docs/reference/logging/)
- [Grafana Loki Documentation](https://grafana.com/docs/loki/latest/)
- [LogQL Query Language](https://grafana.com/docs/loki/latest/logql/)
- [Sentry C++ SDK](https://docs.sentry.io/platforms/native/)
- [Sentry Best Practices](https://docs.sentry.io/product/sentry-basics/integrate-frontend/)
