
        // Rate limiting middleware
        #include <unordered_map>
        #include <chrono>
        #include <string>
        
        class RateLimiter {
        private:
            std::unordered_map<std::string, std::vector<std::chrono::steady_clock::time_point>> requests;
            int max_requests;
            std::chrono::seconds window;
            
        public:
            RateLimiter(int max_req, std::chrono::seconds win) 
                : max_requests(max_req), window(win) {}
                
            bool isAllowed(const std::string& client_id) {
                auto now = std::chrono::steady_clock::now();
                auto& client_requests = requests[client_id];
                
                // Remove old requests
                client_requests.erase(
                    std::remove_if(client_requests.begin(), client_requests.end(),
                        [&](auto time) { return now - time > window; }),
                    client_requests.end()
                );
                
                // Check if under limit
                if (client_requests.size() < max_requests) {
                    client_requests.push_back(now);
                    return true;
                }
                return false;
            }
        };
        