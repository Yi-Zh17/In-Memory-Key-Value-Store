#ifndef MEMPOOL_H_
#define MEMPOOL_H_

#include <vector>

#define MINIMUM_CHUNK_SIZE 8

class MemoryPool
{
private:
    std::vector<std::byte> pool; // Reserved block of memory
    size_t chunk_size; // Size of each chunk
    std::byte* slot; // The next available chunk address
public:
    /**
     * Constructing a memory pool. Allocates a large chunk of memory 
     * and divide it into num_chunks with size chunk_size.
     */
    MemoryPool(size_t num_chunks, size_t chunk_size);
    
    /**
     * Destructor of the MemoryPool class.
     */
    ~MemoryPool();

    /**
     * The allocation function. Returns an address to the next available 
     * chunk, or nullptr if full.
     */
    void* allocate();

    /**
     * Deallocate the memory according to the provided address. Do nothing
     * if the provided address is invalid. Write the original next available 
     * slot into the provided address, and make the address the new next available
     * slot.
     */
    void deallocate(void* chunk);
};

#endif