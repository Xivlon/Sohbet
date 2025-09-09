#include "server/server.h"
#include <iostream>

int main() {
    sohbet::AcademicSocialServer server;
    
    if (!server.initialize()) {
        std::cerr << "Failed to initialize server" << std::endl;
        return 1;
    }
    
    server.run(8080);
    
    return 0;
}