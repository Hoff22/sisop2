# === Compiler & Flags ===
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread

SRC_DIR = src
INC_DIR = include
BIN_DIR = bin
OBJ_DIR = obj

SERVER = server
CLIENT = client

# === Build Modes ===
DEBUG_FLAGS = -g -fsanitize=address -DDEBUG
RELEASE_FLAGS = -O2
PERF_FLAGS = -pg

# === Source Files ===
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
HEADERS := $(wildcard $(INC_DIR)/*.hpp)

CLIENT_SRCS := $(SRC_DIR)/main_client.cpp $(SRC_DIR)/Client.cpp
SERVER_SRCS := $(SRC_DIR)/main_server.cpp $(SRC_DIR)/Server.cpp

COMMON_SRCS := $(filter-out $(CLIENT_SRCS) $(SERVER_SRCS), $(SRCS))

CLIENT_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/debug/%.o,$(CLIENT_SRCS) $(COMMON_SRCS))
SERVER_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/debug/%.o,$(SERVER_SRCS) $(COMMON_SRCS))

# === Targets ===
all: debug

debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: dirs $(BIN_DIR)/debug/$(CLIENT) $(BIN_DIR)/debug/$(SERVER)

release: CXXFLAGS += $(RELEASE_FLAGS)
release: dirs
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) $(COMMON_SRCS) $(CLIENT_SRCS) -o $(BIN_DIR)/release/$(CLIENT)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) $(COMMON_SRCS) $(SERVER_SRCS) -o $(BIN_DIR)/release/$(SERVER)

perf: CXXFLAGS += $(PERF_FLAGS)
perf: dirs
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) $(COMMON_SRCS) $(CLIENT_SRCS) -o $(BIN_DIR)/perf/$(CLIENT)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) $(COMMON_SRCS) $(SERVER_SRCS) -o $(BIN_DIR)/perf/$(SERVER)

# === Object Compilation ===
$(OBJ_DIR)/debug/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

$(BIN_DIR)/debug/$(CLIENT): $(CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BIN_DIR)/debug/$(SERVER): $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# === Utilities ===
dirs:
	@mkdir -p $(BIN_DIR)/debug $(BIN_DIR)/release $(BIN_DIR)/perf
	@mkdir -p $(OBJ_DIR)/debug $(OBJ_DIR)/release $(OBJ_DIR)/perf

clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR)

run-server: debug
	./bin/debug/$(SERVER) 4004

run-client: debug
	./bin/debug/$(CLIENT) 4004
