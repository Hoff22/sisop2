CXX = g++
CXXVER = --std=c++20
CXXFLAGS = -O2 -Wall -Wextra -Wshadow -Wno-unused-result -Wno-sign-compare -fsanitize=undefined -fno-sanitize-recover

all: server client

server: src/server.cpp src/network_utils.cpp
	$(CXX) $(CXXVER) $(CXXFLAGS) -o bin/server src/server.cpp src/network_utils.cpp

client: src/client.cpp src/network_utils.cpp
	$(CXX) $(CXXVER) $(CXXFLAGS) -o bin/client src/client.cpp src/network_utils.cpp

clean:
	rm -rf bin/*
