#include "TCPSocketServer.h"

int main() {
    TCPServer server;
    
    std::thread serverThread([&server]() {
        server.start(8080);
    });
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::string input;
    while (true) {
        std::cout << "Enter message to send (or 'q()' to stop): ";
        std::getline(std::cin, input);
        
        if (input == "q()") {
            break;
        }
        
        server.sendMessage(input);
    }
    
    server.stop();
    if (serverThread.joinable()) {
        serverThread.join();
    }
    
    return 0;
}