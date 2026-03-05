#include <stdexcept>
#include <string>

#include "MemoryPool.h"


MemoryPool::MemoryPool(size_t num_chunks, size_t chunk_size) {
    if (chunk_size < MINIMUM_CHUNK_SIZE) {
        std::string err = "Chunk size must be at least " + std::to_string(MINIMUM_CHUNK_SIZE) + "bytes";
        throw std::invalid_argument(err);
    }

    this->chunk_size = chunk_size;
    pool = std::vector<std::byte> (num_chunks * chunk_size); // Allocate memory for the large chunk

    slot = pool.data(); // Set slot to track the head of raw memory

    // Chain chunks together
    std::byte* current = pool.data();
    std::byte* next;
    for (size_t i = 0; i < num_chunks - 1; i++) {
        next = current + chunk_size;
        std::byte** current_chunk = reinterpret_cast<std::byte**>(current);
        *current_chunk = next;
        current = next;
    }
    std::byte** current_chunk = reinterpret_cast<std::byte**>(current);
    *current_chunk = nullptr; // Attach null pointer at the end
}

MemoryPool::~MemoryPool() {

}

void* MemoryPool::allocate() {
    if (!slot) {
        return nullptr; // Pool is full
    }

    void* addr = slot; // Save the address

    slot = *reinterpret_cast<std::byte**>(slot); // Update the next available slot

    return addr;
}

void MemoryPool::deallocate(void* chunk) {
    if (!chunk) return; // Invalid address

    std::byte** casted_chunk = reinterpret_cast<std::byte**>(chunk);
    *casted_chunk = slot;

    slot = (std::byte*)chunk;
}