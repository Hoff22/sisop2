
CXX = g++
CXXVER = --std=c++20
CXXFLAGS = -O2 -Wall -Wextra -Wshadow -Wno-unused-result -Wno-sign-compare -fsanitize=undefined -fno-sanitize-recover

all: server client

server: server.cpp
	$(CXX) $(CXXVER) $(CXXFLAGS) -o server server.cpp

client: client.cpp
	$(CXX) $(CXXVER) $(CXXFLAGS) -o client client.cpp