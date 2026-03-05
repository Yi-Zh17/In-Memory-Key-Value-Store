#include <cstring>

#include "HashTable.h"

HashTable::HashTable(size_t capacity, MemoryPool* pool) {
    this->capacity = capacity;
    this->pool = pool;

    this->table.resize(capacity);
}

HashTable::~HashTable() {

}

bool HashTable::insert(std::string_view key_view, std::string_view value_view) {
    if (key_view.size() >= KEY_SIZE || value_view.size() >= VALUE_SIZE) {
        return false;
    }

    size_t index = std::hash<std::string_view>{}(key_view) % capacity; // Calculate hash and corresponding index

    // Allocate memory
    void* mem = pool->allocate();
    if (!mem) return false;
    KV* newKV = new (mem) KV();

    // Copy strings
    std::memcpy(newKV->key.data(), key_view.data(), key_view.size());
    std::memcpy(newKV->value.data(), value_view.data(), value_view.size());

    // Add null terminator
    newKV->key[key_view.size()] = '\0';
    newKV->value[value_view.size()] = '\0';

    // Create an Entry
    Entry current_entry{ .state = OCCUPIED, .dib = 0, .data = newKV};

    // Insert into the table
    while(true) {
        if (table[index].state == EMPTY || table[index].state == DELETED) {
            table[index] = current_entry;
            return true;
        } else {
            if (current_entry.dib >= capacity) {
                pool->deallocate(current_entry.data); // Deallocate memory
                return false; // Table is full
            }
            if (std::string_view(table[index].data->key.data()) == key_view) {
                pool->deallocate(table[index].data); // Deallocate old data
                table[index] = current_entry; // Overwrite old value if the keys are the same
                return true;
            }
            if (current_entry.dib > table[index].dib) {
                std::swap(current_entry, table[index]); // Robin Hood hashing
            }
        }
        current_entry.dib++;
        index = (index + 1) % capacity;
    }
}

std::optional<std::string_view> HashTable::get(std::string_view key_view) {
    size_t index = std::hash<std::string_view>{} (key_view) % capacity; // Calculate the index

    int current_distance = 0; // Track the distance

    while(true) {
        if (current_distance >= capacity) {
            return std::nullopt; // Prevent infinite looping
        }
        if (table[index].state == EMPTY) {
            return std::nullopt; // Not found
        }
        if (table[index].state == OCCUPIED 
            && current_distance > table[index].dib) {
            return std::nullopt; // Early exit
        }
        if (table[index].state == OCCUPIED
        && std::string_view(table[index].data->key.data()) == key_view) {
            return std::string_view(table[index].data->value.data()); // Found
        }
        current_distance++;
        index = (index + 1) % capacity;
    }
}

bool HashTable::remove(std::string_view key_view) {
    size_t index = std::hash<std::string_view>{} (key_view) % capacity; // Calculate the index

    int current_distance = 0; // Keep track of the distance

    while(true) {
        if (current_distance > capacity) {
            return false; // Prevent infinite loop
        }
        if (table[index].state == EMPTY) {
            return false;
        }
        if (table[index].state == OCCUPIED
        && current_distance > table[index].dib) {
            return false; // Early exit
        }
        if (table[index].state == OCCUPIED
        && std::string_view(table[index].data->key.data()) == key_view) {
            pool->deallocate(table[index].data); // Deallocate data
            table[index].data = nullptr;
            table[index].state = DELETED;
            return true;
        }
        current_distance++;
        index = (index + 1) % capacity;
    }
}

