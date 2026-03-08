#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest.h"
#include "../HashTable.h"

#define TEST_SIZE 100000
#define CHUNK_SIZE 256

TEST_CASE("Test basic insertion, get, and deletion") {
    HashTable table(TEST_SIZE);

    CHECK(table.insert("player_1", "health: 100") == true); // Insertion
    CHECK(table.get("player_1") == "health: 100"); // Get

    CHECK(table.insert("player_1", "health: 80") == true); // Update
    CHECK(table.get("player_1") == "health: 80");

    CHECK(table.remove("player_1") == true); // Remove
    CHECK(table.get("player_1") == std::nullopt);
}

TEST_CASE("Stress test") {
    MemoryPool pool(TEST_SIZE, CHUNK_SIZE);
    HashTable table(TEST_SIZE);

    std::vector<std::string> vec;
    for (int i = 0; i < TEST_SIZE; i++) {
        vec.push_back(std::to_string(i));
    }

    for (int i = 0; i < TEST_SIZE; i++) {
        CHECK(table.insert(vec[i], vec[i]) == true);
    }

    for (int i = 0; i < TEST_SIZE; i++) {
        CHECK(table.get(vec[i]) == vec[i]);
    }

    for (int i = 0; i < TEST_SIZE; i++) {
        CHECK(table.remove(vec[i]) == true);
    }

    CHECK(pool.get_free_chunks() == TEST_SIZE);
}

TEST_CASE("Overfill") {
    MemoryPool pool(5, CHUNK_SIZE);
    HashTable table(5);

    std::vector<std::string> vec;

    for (int i = 0; i < 6; i++) {
        vec.push_back(std::to_string(i));
    }

    for (int i = 0; i < 5; i++) {
        table.insert(vec[i], vec[i]);
    }

    CHECK(table.insert(vec[5], vec[5]) == false); // Already full
}