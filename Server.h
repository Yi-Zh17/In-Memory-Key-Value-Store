#ifndef SERVER_H_
#define SERVER_H_

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string>

#include "HashTable.h"
#include "ThreadPool.h"


#define BACKLOG 10


class Server {
private:
    int fd; // File Descriptor
    int epoll_fd; // Epoll file descriptor
    uint16_t port; // Port number
    HashTable* table;
    ThreadPool* t_pool;

    // Parse message by delimiter
    std::vector<std::string_view> parseMessage(char* buffer, char delim = ' ');

public:
    Server(uint16_t port, HashTable* table);
    ~Server();

    void start();
};

#endif