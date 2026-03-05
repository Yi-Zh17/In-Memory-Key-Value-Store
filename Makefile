CXX = g++
RM = rm -f
DFLAG = -g
CXXFLAGS = $(DFLAG) -I.

all: test/TestMemoryPool

test/TestMemoryPool: test/TestMemoryPool.cpp MemoryPool.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	$(RM) test/TestMemoryPool