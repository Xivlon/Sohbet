#include "utils/rate_limiter.h"
#include <iostream>
#include <string>

using namespace sohbet::utils;

int main() {
    std::cout << "Rate Limiter Example\n" << std::endl;
    
    // Create a rate limiter: 5 requests/second, burst size of 10
    RateLimiter limiter(5.0, 10);
    
    std::string client_ip = "192.168.1.100";
    
    std::cout << "Simulating requests from IP: " << client_ip << std::endl;
    std::cout << "Configuration: 5 requests/second, burst size of 10\n" << std::endl;
    
    // Try 15 requests in quick succession
    for (int i = 1; i <= 15; i++) {
        bool allowed = limiter.allowRequest(client_ip);
        double remaining = limiter.getRemainingTokens(client_ip);
        
        std::cout << "Request #" << i << ": " 
                  << (allowed ? "ALLOWED" : "BLOCKED")
                  << " (Remaining tokens: " << remaining << ")" << std::endl;
    }
    
    std::cout << "\n--- Resetting rate limit for IP ---\n" << std::endl;
    limiter.resetIP(client_ip);
    
    // Try 5 more requests after reset
    for (int i = 16; i <= 20; i++) {
        bool allowed = limiter.allowRequest(client_ip);
        double remaining = limiter.getRemainingTokens(client_ip);
        
        std::cout << "Request #" << i << ": " 
                  << (allowed ? "ALLOWED" : "BLOCKED")
                  << " (Remaining tokens: " << remaining << ")" << std::endl;
    }
    
    std::cout << "\n--- Multiple IPs Example ---\n" << std::endl;
    
    std::string ip1 = "10.0.0.1";
    std::string ip2 = "10.0.0.2";
    
    // Each IP has independent rate limits
    for (int i = 1; i <= 12; i++) {
        bool allowed1 = limiter.allowRequest(ip1);
        bool allowed2 = limiter.allowRequest(ip2);
        
        std::cout << "Request #" << i 
                  << " - IP1: " << (allowed1 ? "ALLOWED" : "BLOCKED")
                  << ", IP2: " << (allowed2 ? "ALLOWED" : "BLOCKED")
                  << std::endl;
    }
    
    std::cout << "\nTotal IPs tracked: " << limiter.getTrackedIPCount() << std::endl;
    
    return 0;
}
