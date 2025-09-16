#include "TCPSocketServer.h"

int main() {
	std::unique_ptr<TCPServer> server = std::make_unique<TCPServer>();
    //TCPServer server;
    
    std::thread serverThread([&server]() {
        server->serverStart(8080);
    });
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::string input;
    while (true) {
        std::cout << "Enter message to send (or 'q()' to stop): ";
        std::getline(std::cin, input);
        
        if (input == "q()") {
            break;
        }
        
        server->sendMessage(input);
    }
    
    server->serverStop();
    if (serverThread.joinable()) {
        serverThread.join();
    }
    
    return 0;
}