#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <cstdlib>
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

private:
    int server_fd;
    struct sockaddr_in address;
    int port;
    char ip[INET_ADDRSTRLEN];
    std::function<void(const char*)> messageHandler;
    std::function<void()> onDisconnect;
    int connectedClients;

    int getLocalIP();
};

TCPServer::TCPServer(int port, std::function<void(const char*)> messageHandler, std::function<void()> onDisconnect) 
    : port(port), messageHandler(messageHandler), onDisconnect(onDisconnect), connectedClients(0) {
    if (getLocalIP() != 0) {
        std::cerr << "Error getting local IP address" << std::endl;
        exit(EXIT_FAILURE);
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
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
}

TCPServer::~TCPServer() {
    close(server_fd);
}

int TCPServer::getLocalIP() {
    struct ifaddrs *ifaddr, *ifa;
    int family, s;

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return -1;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        family = ifa->ifa_addr->sa_family;

        if (family == AF_INET) {
            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), ip, INET_ADDRSTRLEN, NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                std::cerr << "getnameinfo() failed: " << gai_strerror(s) << std::endl;
                return -1;
            }
            if (strcmp(ifa->ifa_name, "lo") != 0) {
                freeifaddrs(ifaddr);
                return 0;
            }
        }
    }

    freeifaddrs(ifaddr);
    return -1;
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
    }
    --connectedClients;
    checkDisconnect();
}

void TCPServer::checkDisconnect() {
    if (connectedClients == 0) {
        onDisconnect();
    }
}

void handleMessage(const char* message) {
    size_t found = message.find("$TEXT$");
    if (found != std::string::npos) {
        std::cout << "Verilen string içinde '$TEXT$' bulundu." << std::endl;
    } 

}

void handleDisconnect() {
    std::cout << "All clients disconnected" << std::endl;
}

int main() {
    int port = 8080;  // Desired port number
    TCPServer server(port, handleMessage, handleDisconnect);

    while (true) {
        int client_socket = server.acceptConnection();
        std::thread([&server, client_socket]() {
            server.handleClient(client_socket);
        }).detach(); // Detach the thread to allow it to run independently
    }
    return 0;
}
