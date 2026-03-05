#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "../MemoryPool.h"
#include "doctest.h"

#include <iostream>


TEST_CASE("Allocating chunk size less than 8") {
    CHECK_THROWS_AS(MemoryPool(10, 2), std::invalid_argument);
}

TEST_CASE("Allocating and deallocating one element") {
    MemoryPool pool(10, 32);

    void* newAddr = pool.allocate();

    int* my_int = new (newAddr) int(42);

    CHECK(*my_int == 42);

    pool.deallocate(my_int);
}

TEST_CASE("Allocation after the pool is full") {
    MemoryPool pool(10, 32);

    for (size_t i = 0; i < 10; i++) {
        void* newAddr = pool.allocate();
    }

    void* anotherAddr = pool.allocate();
    CHECK(anotherAddr == nullptr);
}
