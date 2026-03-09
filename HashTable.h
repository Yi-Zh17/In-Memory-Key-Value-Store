#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#include <array>
#include <vector>
#include <string>
#include <string_view>
#include <optional>
#include <mutex>
#include <shared_mutex>

#include "MemoryPool.h"

#define KEY_SIZE 64
#define VALUE_SIZE 192

/**
 * The key-value data structure.
 */
struct KV
{
    std::array<char, KEY_SIZE> key;
    std::array<char, VALUE_SIZE> value;
};

/**
 * The states of a slot.
 */
enum SlotState {
    EMPTY, OCCUPIED, DELETED
};

class HashTable {
private:
    struct Entry // The metadata struct
    {
        SlotState state = EMPTY;
        int dib = 0; // Distant from the initial bucket
        KV* data;
    };
    
    std::vector<Entry> table; // Flat array HashTable
    size_t capacity;
    std::shared_mutex rw_lock; // Reader-Writer lock


public:
    HashTable(size_t capacity);
    ~HashTable();

    /**
     * Wrap a key-value pair with metadata and insert it into the table. Returns
     * true if succeeded and false if failed.
     */
    bool insert(std::string_view key_view, std::string_view value_view);

    /**
     * Get the value paired with the provided key. Returns the value if the key
     * exists and nothing if the key does not exist.
     */
    std::optional<std::string_view> get(std::string_view key_view);

    /**
     * Remove the key-value pair from the table. Returns true if succeeded and 
     * false if failed.
     */
    bool remove(std::string_view key_view);

    static size_t get_available_memory(); // Returns the free chunks of the local pool
};

#endif