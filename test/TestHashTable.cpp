#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest.h"
#include "../HashTable.h"

#define TEST_SIZE 10000
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

    CHECK(HashTable::get_available_memory() == TEST_SIZE);
}

TEST_CASE("Overfill") {
    HashTable table(TEST_SIZE);

    std::vector<std::string> vec;

    for (int i = 0; i < TEST_SIZE + 1; i++) {
        vec.push_back(std::to_string(i));
    }

    for (int i = 0; i < TEST_SIZE; i++) {
        table.insert(vec[i], vec[i]);
    }

    CHECK(table.insert(vec[TEST_SIZE], vec[TEST_SIZE]) == false); // Already full
}