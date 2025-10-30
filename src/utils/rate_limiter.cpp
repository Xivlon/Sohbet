#include "utils/rate_limiter.h"
#include <algorithm>

namespace sohbet {
namespace utils {

// ============================================================================
// TokenBucket Implementation
// ============================================================================

TokenBucket::TokenBucket(size_t capacity, double refill_rate)
    : capacity_(capacity),
      refill_rate_(refill_rate),
      tokens_(static_cast<double>(capacity)),
      last_refill_time_(std::chrono::steady_clock::now()) {
}

void TokenBucket::refill() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - last_refill_time_
    ).count() / 1000.0; // Convert to seconds
    
    double tokens_to_add = elapsed * refill_rate_;
    tokens_ = std::min(tokens_ + tokens_to_add, static_cast<double>(capacity_));
    last_refill_time_ = now;
}

bool TokenBucket::consume(size_t tokens) {
    std::lock_guard<std::mutex> lock(mutex_);
    refill();
    
    if (tokens_ >= static_cast<double>(tokens)) {
        tokens_ -= static_cast<double>(tokens);
        return true;
    }
    return false;
}

double TokenBucket::getTokens() const {
    std::lock_guard<std::mutex> lock(mutex_);
    // We need to refill without modifying the object
    // Since this is const, we'll just calculate what the tokens would be
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - last_refill_time_
    ).count() / 1000.0;
    
    double tokens_to_add = elapsed * refill_rate_;
    return std::min(tokens_ + tokens_to_add, static_cast<double>(capacity_));
}

void TokenBucket::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    tokens_ = static_cast<double>(capacity_);
    last_refill_time_ = std::chrono::steady_clock::now();
}

// ============================================================================
// RateLimiter Implementation
// ============================================================================

RateLimiter::RateLimiter(double requests_per_second, size_t burst_size)
    : requests_per_second_(requests_per_second),
      burst_size_(burst_size) {
}

bool RateLimiter::allowRequest(const std::string& ip_address, size_t tokens) {
    if (ip_address.empty()) {
        return false;
    }
    
    // Get the bucket pointer while holding the lock
    TokenBucket* bucket = nullptr;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = ip_buckets_.find(ip_address);
        if (it == ip_buckets_.end()) {
            // Create new bucket for this IP
            auto result = ip_buckets_.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(ip_address),
                std::forward_as_tuple(burst_size_, requests_per_second_)
            );
            bucket = result.first->second.bucket.get();
            result.first->second.last_access = std::chrono::steady_clock::now();
        } else {
            bucket = it->second.bucket.get();
            it->second.last_access = std::chrono::steady_clock::now();
        }
    }
    // Lock is released here, then we call bucket->consume()
    return bucket->consume(tokens);
}

double RateLimiter::getRemainingTokens(const std::string& ip_address) {
    if (ip_address.empty()) {
        return 0.0;
    }
    
    TokenBucket* bucket = nullptr;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = ip_buckets_.find(ip_address);
        if (it == ip_buckets_.end()) {
            // No bucket exists yet, so full capacity is available
            return static_cast<double>(burst_size_);
        }
        
        bucket = it->second.bucket.get();
        it->second.last_access = std::chrono::steady_clock::now();
    }
    // Lock is released here, then we call bucket->getTokens()
    return bucket->getTokens();
}

void RateLimiter::resetIP(const std::string& ip_address) {
    TokenBucket* bucket = nullptr;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = ip_buckets_.find(ip_address);
        if (it != ip_buckets_.end()) {
            bucket = it->second.bucket.get();
            it->second.last_access = std::chrono::steady_clock::now();
        }
    }
    // Lock is released here, then we call bucket->reset()
    if (bucket) {
        bucket->reset();
    }
}

void RateLimiter::clearAll() {
    std::lock_guard<std::mutex> lock(mutex_);
    ip_buckets_.clear();
}

void RateLimiter::cleanup(std::chrono::seconds inactive_duration) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::chrono::steady_clock::now();
    
    // Remove IPs that haven't been accessed for the specified duration
    for (auto it = ip_buckets_.begin(); it != ip_buckets_.end();) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - it->second.last_access
        );
        
        if (elapsed >= inactive_duration) {
            it = ip_buckets_.erase(it);
        } else {
            ++it;
        }
    }
}

size_t RateLimiter::getTrackedIPCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return ip_buckets_.size();
}

} // namespace utils
} // namespace sohbet
