
CXX = g++
CXXVER = --std=c++20
CXXFLAGS = -O2 -Wall -Wextra -Wshadow -Wno-unused-result -Wno-sign-compare -fsanitize=undefined -fno-sanitize-recover

all: server client

server: src/server.cpp
	$(CXX) $(CXXVER) $(CXXFLAGS) -o bin/server src/server.cpp

client: src/client.cpp
	$(CXX) $(CXXVER) $(CXXFLAGS) -o bin/client src/client.cpp

clean:
	rm -rf bin/*