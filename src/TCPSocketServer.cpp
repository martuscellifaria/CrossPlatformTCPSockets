#include "TCPSocketServer.h"

TCPServer::TCPServer() {
    m_iServerSocket = INVALID_SOCKET;
    m_iClientSocket = INVALID_SOCKET;
    m_bRunning = false;
    initializeNetwork();
}

TCPServer::~TCPServer() {
    if (m_bRunning){
        serverStop();
    }
    cleanupNetwork();
}


void TCPServer::initializeNetwork() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }
#endif
}

void TCPServer::readThreadFunction() {
    char buffer[1024];
    while (m_bRunning) {
        if (m_iClientSocket == INVALID_SOCKET) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        int bytesReceived = recv(m_iClientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::lock_guard<std::mutex> lock(m_mConsoleMutex);
            processMessage(buffer);
        } else if (bytesReceived == 0) {
            std::lock_guard<std::mutex> lock(m_mConsoleMutex);
            std::cout << "Client disconnected" << '\n';
            close(m_iClientSocket);
            m_iClientSocket = INVALID_SOCKET;
        } else {
            if (!m_bRunning) break;
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void TCPServer::processMessage(const char* cTelegram) {
    std::cout << "Received " << cTelegram << '\n';
}

bool TCPServer::serverStart(int port) {
    m_iServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_iServerSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed" << '\n';
        return false;
    }

    int opt = 1;
#ifdef _WIN32
    setsockopt(m_iServerSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
#else
    setsockopt(m_iServerSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(m_iServerSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed" << '\n';
        return false;
    }

    if (listen(m_iServerSocket, 1) == SOCKET_ERROR) {
        std::cerr << "Listen failed" << '\n';
        return false;
    }

    std::cout << "Server started on port " << port << '\n';
    m_bRunning = true;
    
    m_tReadThread = std::thread(&TCPServer::readThreadFunction, this);
    while (m_bRunning) {
        std::cout << "Waiting for client connection..." << '\n';
        
        sockaddr_in clientAddr;
#ifdef _WIN32
        int clientAddrSize = sizeof(clientAddr);
#else
        socklen_t clientAddrSize = sizeof(clientAddr);
#endif

        SOCKET newClient = accept(m_iServerSocket, (sockaddr*)&clientAddr, &clientAddrSize);
        if (newClient == INVALID_SOCKET) {
            if (m_bRunning) {
                std::cerr << "Accept failed" << '\n';
            }
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(m_mConsoleMutex);
            std::cout << "Client connected: " << inet_ntoa(clientAddr.sin_addr) 
                        << ":" << ntohs(clientAddr.sin_port) << '\n';
        }

        m_iClientSocket = newClient;
    }

    return true;
}

void TCPServer::serverStop() {
    m_bRunning = false;
    if (m_iClientSocket != INVALID_SOCKET) {
        shutdown(m_iClientSocket, SHUT_RDWR);
        close(m_iClientSocket);
    }
    
    if (m_iServerSocket != INVALID_SOCKET) {
        shutdown(m_iServerSocket, SHUT_RDWR);
        close(m_iServerSocket);
    }

    if (m_tReadThread.joinable()) {
        m_tReadThread.join();
    }
}

bool TCPServer::sendMessage(const std::string& message) {
    if (m_iClientSocket == INVALID_SOCKET) {
        std::cerr << "No client connected" << '\n';
        return false;
    }

    int result = send(m_iClientSocket, message.c_str(), message.length(), 0);
    if (result == SOCKET_ERROR) {
        std::cerr << "Send failed" << '\n';
        return false;
    }
    
    return true;
}

void TCPServer::cleanupNetwork() {
#ifdef _WIN32
    WSACleanup();
#endif
}