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

class TCPServer {
public:
    TCPServer(int port, std::function<void(const char*)> messageHandler);
    ~TCPServer();
    int acceptConnection();
    void handleClient(int clientSocket);

private:
    int server_fd;
    struct sockaddr_in address;
    int port;
    char ip[INET_ADDRSTRLEN];
    std::function<void(const char*)> messageHandler;

    int getLocalIP();
};

TCPServer::TCPServer(int port, std::function<void(const char*)> messageHandler) : port(port), messageHandler(messageHandler) {
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

    std::cout << "Server listening on IP: " << ip << ", Port: " << port << std::endl;
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

            // We only take the first non-loopback address
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
    char buffer[1024] = {0};
    while (true) {
        int valread = read(clientSocket, buffer, 1024);
        if (valread == 0) {
            std::cout << "Client disconnected" << std::endl;
            break;
        } else if (valread < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        std::cout << "Message from client: " << buffer << std::endl;
        // Handle message using external function
        messageHandler(buffer);
    }
}

void handleMessage(const char* message) {
    std::cout << "External function handling message: " << message << std::endl;
}

int main() {
    int port = 8080;  // Desired port number
    TCPServer server(port, handleMessage);

    while (true) {
        int client_socket = server.acceptConnection();
        std::cout << "Connection accepted" << std::endl;

        // Handle client communication in a new thread
        std::thread([&server, client_socket]() {
            server.handleClient(client_socket);
        }).detach(); // Detach the thread to allow it to run independently
    }

    return 0;
}
