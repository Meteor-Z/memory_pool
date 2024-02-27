CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -O2 -g

memory_pool: main.cc
	${CXX} ${CXXFLAGS} main.cc -o memory_pool

.PHONY: clean
clean: 
	rm -f memory_pool