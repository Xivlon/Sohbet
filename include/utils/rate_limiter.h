#pragma once

#include <string>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <memory>

namespace sohbet {
namespace utils {

/**
 * Token bucket for tracking request rates per IP
 */
class TokenBucket {
public:
    /**
     * Constructor
     * @param capacity Maximum number of tokens (requests)
     * @param refill_rate Number of tokens to add per second
     */
    TokenBucket(size_t capacity, double refill_rate);
    
    /**
     * Try to consume a token
     * @param tokens Number of tokens to consume (default: 1)
     * @return true if tokens were consumed, false if not enough tokens available
     */
    bool consume(size_t tokens = 1);
    
    /**
     * Get current number of tokens
     * @return Current token count
     */
    double getTokens() const;
    
    /**
     * Reset the bucket to full capacity
     */
    void reset();

private:
    size_t capacity_;
    double refill_rate_;
    double tokens_;
    std::chrono::steady_clock::time_point last_refill_time_;
    mutable std::mutex mutex_;
    
    /**
     * Refill tokens based on elapsed time
     */
    void refill();
};

/**
 * Rate limiter with IP tracking and token bucket algorithm
 * Thread-safe implementation for concurrent request handling
 */
class RateLimiter {
public:
    /**
     * Constructor with default settings
     * @param requests_per_second Maximum requests per second per IP (default: 10)
     * @param burst_size Maximum burst size (default: 20)
     */
    explicit RateLimiter(
        double requests_per_second = 10.0,
        size_t burst_size = 20
    );
    
    /**
     * Check if a request from an IP should be allowed
     * @param ip_address IP address of the client
     * @param tokens Number of tokens to consume (default: 1)
     * @return true if request is allowed, false if rate limit exceeded
     */
    bool allowRequest(const std::string& ip_address, size_t tokens = 1);
    
    /**
     * Get remaining tokens for an IP
     * @param ip_address IP address of the client
     * @return Number of tokens remaining
     */
    double getRemainingTokens(const std::string& ip_address);
    
    /**
     * Reset rate limit for a specific IP
     * @param ip_address IP address to reset
     */
    void resetIP(const std::string& ip_address);
    
    /**
     * Clear all rate limit data
     */
    void clearAll();
    
    /**
     * Clean up old IP entries to prevent memory leaks
     * Removes IPs that haven't been accessed recently
     * @param inactive_duration Duration of inactivity before cleanup (default: 1 hour)
     */
    void cleanup(std::chrono::seconds inactive_duration = std::chrono::hours(1));
    
    /**
     * Get total number of tracked IPs
     * @return Number of IP addresses being tracked
     */
    size_t getTrackedIPCount() const;

private:
    double requests_per_second_;
    size_t burst_size_;
    
    struct IPBucketData {
        std::unique_ptr<TokenBucket> bucket;
        std::chrono::steady_clock::time_point last_access;
        
        IPBucketData(size_t capacity, double refill_rate)
            : bucket(std::make_unique<TokenBucket>(capacity, refill_rate)),
              last_access(std::chrono::steady_clock::now()) {}
    };
    
    std::unordered_map<std::string, IPBucketData> ip_buckets_;
    mutable std::mutex mutex_;
};

} // namespace utils
} // namespace sohbet
