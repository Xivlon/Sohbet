#include "utils/rate_limiter.h"
#include <iostream>
#include <cassert>
#include <thread>
#include <vector>
#include <atomic>

using namespace sohbet::utils;

void testTokenBucketBasic() {
    std::cout << "Testing TokenBucket basic functionality..." << std::endl;
    
    // Create a bucket with capacity 5, refill rate 1 token/second
    TokenBucket bucket(5, 1.0);
    
    // Should have full capacity initially
    assert(bucket.getTokens() >= 4.9 && bucket.getTokens() <= 5.0);
    
    // Consume 3 tokens
    assert(bucket.consume(3) == true);
    
    // Should have ~2 tokens left
    assert(bucket.getTokens() >= 1.9 && bucket.getTokens() <= 2.1);
    
    // Try to consume 3 more - should fail
    assert(bucket.consume(3) == false);
    
    // Should still have ~2 tokens
    assert(bucket.getTokens() >= 1.9 && bucket.getTokens() <= 2.1);
    
    // Reset should fill bucket
    bucket.reset();
    assert(bucket.getTokens() >= 4.9 && bucket.getTokens() <= 5.0);
    
    std::cout << "TokenBucket basic test passed!" << std::endl;
}

void testTokenBucketRefill() {
    std::cout << "Testing TokenBucket refill..." << std::endl;
    
    // Create a bucket with capacity 10, refill rate 5 tokens/second
    TokenBucket bucket(10, 5.0);
    
    // Consume 8 tokens
    assert(bucket.consume(8) == true);
    assert(bucket.getTokens() >= 1.9 && bucket.getTokens() <= 2.1);
    
    // Wait 1 second - should refill 5 tokens
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // Should have ~7 tokens now (2 + 5)
    double tokens = bucket.getTokens();
    assert(tokens >= 6.5 && tokens <= 7.5);
    
    // Wait another second - should refill to capacity (10)
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    tokens = bucket.getTokens();
    assert(tokens >= 9.5 && tokens <= 10.0);
    
    std::cout << "TokenBucket refill test passed!" << std::endl;
}

void testRateLimiterBasic() {
    std::cout << "Testing RateLimiter basic functionality..." << std::endl;
    
    // Create rate limiter: 5 requests/second, burst size 10
    RateLimiter limiter(5.0, 10);
    
    // First request from IP should be allowed
    assert(limiter.allowRequest("192.168.1.1") == true);
    
    // Check remaining tokens
    double remaining = limiter.getRemainingTokens("192.168.1.1");
    assert(remaining >= 8.9 && remaining <= 9.1);
    
    // Multiple requests within burst size should succeed
    for (int i = 0; i < 8; i++) {
        assert(limiter.allowRequest("192.168.1.1") == true);
    }
    
    // Now should have ~1 token left
    remaining = limiter.getRemainingTokens("192.168.1.1");
    assert(remaining >= 0.9 && remaining <= 1.1);
    
    // One more should work
    assert(limiter.allowRequest("192.168.1.1") == true);
    
    // Now should be rate limited
    assert(limiter.allowRequest("192.168.1.1") == false);
    
    std::cout << "RateLimiter basic test passed!" << std::endl;
}

void testRateLimiterIPTracking() {
    std::cout << "Testing RateLimiter IP tracking..." << std::endl;
    
    RateLimiter limiter(10.0, 5);
    
    // Different IPs should have independent rate limits
    assert(limiter.allowRequest("192.168.1.1") == true);
    assert(limiter.allowRequest("192.168.1.2") == true);
    assert(limiter.allowRequest("10.0.0.1") == true);
    
    // Verify 3 IPs are tracked
    assert(limiter.getTrackedIPCount() == 3);
    
    // Exhaust first IP
    for (int i = 0; i < 10; i++) {
        limiter.allowRequest("192.168.1.1");
    }
    
    // First IP should be rate limited
    assert(limiter.allowRequest("192.168.1.1") == false);
    
    // Other IPs should still work
    assert(limiter.allowRequest("192.168.1.2") == true);
    assert(limiter.allowRequest("10.0.0.1") == true);
    
    std::cout << "RateLimiter IP tracking test passed!" << std::endl;
}

void testRateLimiterReset() {
    std::cout << "Testing RateLimiter reset..." << std::endl;
    
    RateLimiter limiter(5.0, 3);
    
    // Exhaust the rate limit
    assert(limiter.allowRequest("192.168.1.1") == true);
    assert(limiter.allowRequest("192.168.1.1") == true);
    assert(limiter.allowRequest("192.168.1.1") == true);
    assert(limiter.allowRequest("192.168.1.1") == false);
    
    // Reset the IP
    limiter.resetIP("192.168.1.1");
    
    // Should work again
    assert(limiter.allowRequest("192.168.1.1") == true);
    assert(limiter.allowRequest("192.168.1.1") == true);
    
    std::cout << "RateLimiter reset test passed!" << std::endl;
}

void testRateLimiterClearAll() {
    std::cout << "Testing RateLimiter clearAll..." << std::endl;
    
    RateLimiter limiter(5.0, 5);
    
    // Add multiple IPs
    limiter.allowRequest("192.168.1.1");
    limiter.allowRequest("192.168.1.2");
    limiter.allowRequest("192.168.1.3");
    
    assert(limiter.getTrackedIPCount() == 3);
    
    // Clear all
    limiter.clearAll();
    
    assert(limiter.getTrackedIPCount() == 0);
    
    // All IPs should work again with fresh limits
    assert(limiter.allowRequest("192.168.1.1") == true);
    assert(limiter.getTrackedIPCount() == 1);
    
    std::cout << "RateLimiter clearAll test passed!" << std::endl;
}

void testRateLimiterCleanup() {
    std::cout << "Testing RateLimiter cleanup..." << std::endl;
    
    RateLimiter limiter(10.0, 10);
    
    // Add some IPs
    limiter.allowRequest("192.168.1.1");
    limiter.allowRequest("192.168.1.2");
    limiter.allowRequest("192.168.1.3");
    
    assert(limiter.getTrackedIPCount() == 3);
    
    // Wait 2 seconds
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    
    // Add a new IP to keep it fresh
    limiter.allowRequest("192.168.1.4");
    
    // Cleanup IPs inactive for 1 second
    limiter.cleanup(std::chrono::seconds(1));
    
    // Should only have the new IP
    assert(limiter.getTrackedIPCount() == 1);
    
    std::cout << "RateLimiter cleanup test passed!" << std::endl;
}

void testRateLimiterEmptyIP() {
    std::cout << "Testing RateLimiter with empty IP..." << std::endl;
    
    RateLimiter limiter(10.0, 10);
    
    // Empty IP should be rejected
    assert(limiter.allowRequest("") == false);
    assert(limiter.getRemainingTokens("") == 0.0);
    assert(limiter.getTrackedIPCount() == 0);
    
    std::cout << "RateLimiter empty IP test passed!" << std::endl;
}

void testRateLimiterMultipleTokens() {
    std::cout << "Testing RateLimiter with multiple tokens..." << std::endl;
    
    RateLimiter limiter(10.0, 10);
    
    // Consume 5 tokens at once
    assert(limiter.allowRequest("192.168.1.1", 5) == true);
    
    // Should have ~5 tokens left
    double remaining = limiter.getRemainingTokens("192.168.1.1");
    assert(remaining >= 4.9 && remaining <= 5.1);
    
    // Try to consume 6 tokens - should fail
    assert(limiter.allowRequest("192.168.1.1", 6) == false);
    
    // Consume 3 tokens - should work
    assert(limiter.allowRequest("192.168.1.1", 3) == true);
    
    // Should have ~2 tokens left
    remaining = limiter.getRemainingTokens("192.168.1.1");
    assert(remaining >= 1.9 && remaining <= 2.1);
    
    std::cout << "RateLimiter multiple tokens test passed!" << std::endl;
}

void testRateLimiterConcurrency() {
    std::cout << "Testing RateLimiter concurrency..." << std::endl;
    
    // Use a slower refill rate to minimize token refill during test
    RateLimiter limiter(1.0, 50);
    
    // Create multiple threads making requests
    std::vector<std::thread> threads;
    std::atomic<int> success_count(0);
    std::atomic<int> blocked_count(0);
    
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([&]() {
            std::string ip = "192.168.1." + std::to_string(i % 3);
            for (int j = 0; j < 20; j++) {
                if (limiter.allowRequest(ip)) {
                    success_count.fetch_add(1);
                } else {
                    blocked_count.fetch_add(1);
                }
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "Total successful requests: " << success_count << std::endl;
    std::cout << "Total blocked requests: " << blocked_count << std::endl;
    
    // Verify that:
    // 1. Some requests succeeded
    assert(success_count > 0);
    // 2. Some requests were blocked (proves rate limiting is working)
    assert(blocked_count > 0);
    // 3. Total requests is as expected
    assert(success_count + blocked_count == 200);
    
    std::cout << "RateLimiter concurrency test passed!" << std::endl;
}

int main() {
    std::cout << "Running RateLimiter tests..." << std::endl << std::endl;
    
    testTokenBucketBasic();
    testTokenBucketRefill();
    testRateLimiterBasic();
    testRateLimiterIPTracking();
    testRateLimiterReset();
    testRateLimiterClearAll();
    testRateLimiterCleanup();
    testRateLimiterEmptyIP();
    testRateLimiterMultipleTokens();
    testRateLimiterConcurrency();
    
    std::cout << std::endl << "All RateLimiter tests passed!" << std::endl;
    return 0;
}
