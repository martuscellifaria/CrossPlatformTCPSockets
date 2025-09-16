#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #define close closesocket
    #define SHUT_RDWR SD_BOTH
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    #define SOCKET int
    #define INVALID_SOCKET (SOCKET)(~0)
    #define SOCKET_ERROR (-1)
#endif

class TCPServer
{
public:
    TCPServer();
    virtual ~TCPServer();
private:
    void initializeNetwork();
    void cleanupNetwork();
    void readThreadFunction();
    void processMessage(const char* cTelegram);

public:
    bool start(int port);
    void stop();
    bool sendMessage(const std::string& sMessage);

private:
    SOCKET m_iServerSocket;
    SOCKET m_iClientSocket;
    std::thread m_tReadThread;
    std::atomic<bool> m_bRunning;
    std::mutex m_mConsoleMutex;
};