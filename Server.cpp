#include <stdexcept>
#include <unistd.h>
#include <cstdlib>

#include "Server.h"
#include "ThreadPool.h"
#include "Logger.h"

constexpr int MAX_EVENTS = 64; // Maximum events in epoll


Server::Server(uint16_t port, HashTable* table) {
    this->port = port;
    this->table = table;

    this->t_pool = new ThreadPool(table);

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

    // Set socket to non-blocking
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    // Create epoll
    this->epoll_fd = epoll_create1(0);

    // Register socket
    struct epoll_event ev;
    ev.events = EPOLLIN; // Alert when there is incoming data
    ev.data.fd = this->fd;
    epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, this->fd, &ev);
}

Server::~Server() {
    close(fd);
    close(epoll_fd);
    delete(this->t_pool);
}

void Server::start() {
    Logger::log(INFO, "Server started");

    // Event buffer
    struct epoll_event events[MAX_EVENTS];

    while (true) {
        int num_events = epoll_wait(this->epoll_fd, events, MAX_EVENTS, -1);

        // Dispatcher loop
        for (int i = 0; i < num_events; i++) {
            if (events[i].data.fd == this->fd) { // New client
                int client_fd = accept(this->fd, nullptr, nullptr);

                int client_fl = fcntl(client_fd, F_GETFL, 0);
                fcntl(client_fd, F_SETFL, client_fl | O_NONBLOCK); // Set client to be non-blocking

                // Register client
                struct epoll_event client_ev;
                client_ev.events = EPOLLIN;
                client_ev.data.fd = client_fd;
                epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, client_fd, &client_ev);
            }
            else { // Existing client
                int client_fd = events[i].data.fd;

                // Read incoming message
                char buffer[1024] = { 0 };
                ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1); 

                if (bytes_read <= 0) { // Empty message
                    close(client_fd);
                    continue;
                }
                
                if (*buffer != '*') { // Only trim user message
                    ssize_t i = bytes_read - 1;
                    while (i >= 0 && (buffer[i] == '\n' || buffer[i] == '\r')) { // Trim incoming message
                        buffer[i] = '\0';
                        i--;
                    }
                }
                
                auto tokens = callParse(buffer); // Parse message

                // Copy strings
                std::vector<std::string> safe_tokens;
                for (int i = 0; i < tokens.size(); i++) {
                    safe_tokens.push_back(std::string(tokens[i]));
                }

                // Create a Task
                Task t; 
                t.client_fd = client_fd;
                t.command = std::move(safe_tokens);

                // Push to queue
                t_pool->enqueue(std::move(t));
            }
        }
    }
}

std::vector<std::string_view> Server::parseMessage(char* buffer, char delim) {
    char* ptr = buffer; // Sliding raw pointer
    char* head = buffer; // Track the start of a string
    std::vector<std::string_view> result;

    while (*ptr != '\0') {
        if (*ptr == delim) {
            if ((ptr - head) > 0) {
                result.push_back(std::string_view(head, ptr - head));
            }
            head = ptr + 1;
        }
        ptr++;
    }

    if ((ptr - head) > 0) {
        result.push_back(std::string_view(head, ptr - head));
    }

    return result;
}

std::vector<std::string_view> Server::parseRESP(char* buffer) {
    char* ptr = buffer;

    std::vector<std::string_view> result;

    while (*ptr != '*' && *ptr != '\0') {
        ptr++; // Slide to the first '*'
    }

    if (*ptr == '\0' || *(ptr+1) == '\0') {
        return result;
    }

    ptr++; // Slide past '*'

    char* end;
    const long argLen = std::strtol(ptr, &end, 10); // Number of arguments

    ptr = end;

    for (long i = 0; i < argLen; i++) {
        while(*ptr != '$' && *ptr != '\0') {
            ptr++; // Slide to number
        }

        if (*ptr == '\0' || *(ptr+1) == '\0') {
            return result;
        }

        ptr++;
        char* argEnd;
        const long len = std::strtol(ptr, &argEnd, 10);

        ptr = argEnd;

        if (*(ptr) == '\r' && *(ptr+1) == '\n') {
            ptr += 2; // Skip \r\n
            result.push_back(std::string_view(ptr, len));
            ptr += len;
        } else {
            return result;
        }
    }

    return result;
}

std::vector<std::string_view> Server::callParse(char* buffer) {
    if (*buffer == '*') {
        return parseRESP(buffer);
    } else {
        return parseMessage(buffer);
    }
}