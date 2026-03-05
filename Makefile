CXX = g++
RM = rm -f
DFLAG = -g
CXXFLAGS = $(DFLAG) --std=c++20 -I.

all: test/TestMemoryPool test/TestHashTable test/TestServer

test/TestMemoryPool: test/TestMemoryPool.cpp MemoryPool.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

test/TestHashTable: test/TestHashTable.cpp HashTable.cpp MemoryPool.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

test/TestServer: test/TestServer.cpp HashTable.cpp MemoryPool.cpp Server.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	$(RM) test/TestMemoryPool