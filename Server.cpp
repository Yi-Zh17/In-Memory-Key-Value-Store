#include <stdexcept>
#include <unistd.h>
#include "Server.h"

constexpr int MAX_EVENTS = 64; // Maximum events in epoll


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
}

void Server::start() {
    log(INFO, "Server started");

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

                ssize_t i = bytes_read - 1;
                while (i >= 0 && (buffer[i] == '\n' || buffer[i] == '\r')) { // Trim incoming message
                    buffer[i] = '\0';
                    i--;
                }

                auto tokens = parseMessage(buffer); // Parse message

                // Check tokens
                if (tokens.empty()) {
                    log(ERROR, "Empty message");
                    close(client_fd);
                    continue; // Jump back to top
                }

                // Perform actions
                if (tokens[0] == "SET" && tokens.size() == 3) { // SET
                    if (!table->insert(tokens[1], tokens[2])) {
                        log(WARNING, "Data not inserted");
                        char msg[] = "Insertion error\n";
                        write(client_fd, msg, std::string_view(msg).size());
                    }
                    else {
                        log(INFO, "Inserted 1 entry");
                        write(client_fd, "OK\n", 3);
                    }
                }
                else if (tokens[0] == "GET" && tokens.size() == 2) { // GET
                    auto result = table->get(tokens[1]);
                    if (result.has_value()) {
                        log(INFO, "Got 1 entry");
                        write(client_fd, result->data(), result->size());
                        write(client_fd, "\n", 1);
                    }
                    else {
                        log(WARNING, "Get query failed");
                        char msg[] = "Not found\n";
                        write(client_fd, msg, std::string_view(msg).size());
                    }
                }
                else if (tokens[0] == "DEL" && tokens.size() == 2) { // DEL
                    if (!table->remove(tokens[1])) {
                        log(ERROR, "Entry not deleted");
                        char msg[] = "Deletion error\n";
                        write(client_fd, msg, std::string_view(msg).size());
                    }
                    else {
                        log(INFO, "Deleted 1 entry");
                        write(client_fd, "OK\n", 3);
                    }
                }
                else {
                    log(ERROR, "Invalid request received");
                    char msg[] = "Invalid request\n";
                    write(client_fd, msg, std::string_view(msg).size());
                    close(client_fd);
                    continue;
                }
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
