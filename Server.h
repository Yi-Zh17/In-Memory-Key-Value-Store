#ifndef SERVER_H_
#define SERVER_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <ctime>
#include <string>
#include <iostream>

#include "HashTable.h"

#define BACKLOG 10


enum LogLevel { DEBUG, INFO, WARNING, ERROR, CRITICAL };

class Server {
private:
    int fd; // File Descriptor
    uint16_t port; // Port number
    HashTable* table;

    // Logs a message with a given log level
    void log(LogLevel level, const std::string& message);
    // Convert Level enum to string
    std::string levelToString(LogLevel level);

public:
    Server(uint16_t port, HashTable* table);
    ~Server();

    void start();
};

#endif