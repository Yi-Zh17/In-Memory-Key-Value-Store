#include <stdexcept>
#include <unistd.h>
#include "Server.h"


void Server::log(LogLevel level, const std::string& message) {
    // Get current timestamp
    time_t now = time(0);
    tm* timeinfo = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp),
        "%Y-%m-%d %H:%M:%S", timeinfo);

    // Log to console
    std::cout << "[" << timestamp << "] "
        << levelToString(level) << ": " << message
        << std::endl;
}

std::string Server::levelToString(LogLevel level) {
    switch (level) {
    case DEBUG:
        return "DEBUG";
    case INFO:
        return "INFO";
    case WARNING:
        return "WARNING";
    case ERROR:
        return "ERROR";
    case CRITICAL:
        return "CRITICAL";
    default:
        return "UNKNOWN";
    }
}

Server::Server(uint16_t port, HashTable* table) {
    this->port = port;
    this->table = table;

    this->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    // Reuse address immediately if crashed
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Fill in the address struct
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind socket to address
    if (bind(fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        throw std::runtime_error("Failed to bind socket to address");
    }

    // Starting listening
    if (listen(fd, BACKLOG) < 0) {
        throw std::runtime_error("Failed to listen");
    }
}

Server::~Server() {
    close(fd);
}

void Server::start() {
    log(INFO, "Server started");
    while(true) {
        int client_fd = accept(fd, nullptr, nullptr); // Accept the client
        if (client_fd < 0) {
            log(WARNING, "Failed to accept client");
            continue;
        }

        char buffer[1024] = {0};
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1); // Read incoming message

        ssize_t i = bytes_read - 1;
        while(i >= 0 && (buffer[i] == '\n' || buffer[i] == '\r')) { // Trim incoming message
            buffer[i] = '\0';
            i--;
        }

        std::string message = std::string("Message received: ") + std::string(buffer);
        log(INFO, message);

        write(client_fd, "ACK\n", 4); // Acknowledge

        close(client_fd);
    }
}
