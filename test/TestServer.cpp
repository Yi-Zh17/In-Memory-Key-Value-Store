#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest.h"
#include "Server.h"
#include "../HashTable.h"

#define DEFAULT_TEST_SIZE 100000
#define CHUNK_SIZE 256
#define DEFAULT_PORT 8080

TEST_CASE("Run server") {
    MemoryPool pool(DEFAULT_TEST_SIZE, CHUNK_SIZE);
    HashTable table(DEFAULT_TEST_SIZE, &pool);
    Server server(DEFAULT_PORT, &table);

    server.start();
}