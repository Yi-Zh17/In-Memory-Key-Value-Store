#include "Server.h"
#include "../HashTable.h"

#define DEFAULT_TEST_SIZE 1000000
#define CHUNK_SIZE 256
#define DEFAULT_PORT 8080

int main() {
    // Initialise database and server
    HashTable table(DEFAULT_TEST_SIZE);
    Server server(DEFAULT_PORT, &table);

    // Start server
    server.start();
}