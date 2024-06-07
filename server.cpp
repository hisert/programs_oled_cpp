#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <vector>
#include <functional>
#include <string>

class TCPServer {
public:
    TCPServer(int port, std::function<void(const char*)> messageHandler, std::function<void()> onDisconnect);
    ~TCPServer();
    int acceptConnection();
    void handleClient(int clientSocket);
    void checkDisconnect();
    void run();

private:
    int server_fd;
    struct sockaddr_in address;
    int port;
    char ip[INET_ADDRSTRLEN] = "127.0.0.1"; // IP adresini burada sabitliyoruz
    std::function<void(const char*)> messageHandler;
    std::function<void()> onDisconnect;
    int connectedClients;
    std::thread serverThread;
};

TCPServer::TCPServer(int port, std::function<void(const char*)> messageHandler, std::function<void()> onDisconnect)
    : port(port), messageHandler(messageHandler), onDisconnect(onDisconnect), connectedClients(0) {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    // SO_REUSEADDR seçeneği ayarlanıyor
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip);
    address.sin_port = htons(port);
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    std::cout << "Oled Server listening on IP: " << ip << ", Port: " << port << std::endl;

    // Thread'i başlat
    serverThread = std::thread(&TCPServer::run, this);
}

TCPServer::~TCPServer() {
    close(server_fd);
    // Thread'i sonlandır
    serverThread.join();
}

void TCPServer::run() {
    while (true) {
        int client_socket = acceptConnection();
        std::thread([&client_socket, this]() {
            handleClient(client_socket);
        }).detach();
    }
}

int TCPServer::acceptConnection() {
    int addrlen = sizeof(address);
    int new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    if (new_socket < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    return new_socket;
}

void TCPServer::handleClient(int clientSocket) {
    ++connectedClients;
    char buffer[1024] = {0};
    while (true) {
        int valread = read(clientSocket, buffer, 1024);
        if (valread == 0) {
            break;
        } else if (valread < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        messageHandler(buffer);
        for(int x=0;x<1024;x++) buffer[x] = 0;
    }
    --connectedClients;
    checkDisconnect();
}

void TCPServer::checkDisconnect() {
    if (connectedClients == 0) {
        onDisconnect();
    }
}

int main() {
    auto messageHandler = [](const char* message) {
        std::cout << "Received: " << message << std::endl;
    };

    auto onDisconnect = []() {
        std::cout << "Client disconnected" << std::endl;
    };

    TCPServer server(8080, messageHandler, onDisconnect);

    // Sunucunun bitmesini beklemek için ana thread'i engelle
    while (true) {
        sleep(1);
    }

    return 0;
}
