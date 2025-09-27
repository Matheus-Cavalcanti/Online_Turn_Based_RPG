#Compilator
CXX         := g++
#Compilator flags: C++17, include 'headers' root dir and show all warnings
CXXFLAGS    := -std=c++17 -I./headers -Wall

#Linker flags:
LDFLAGS_SERVER := -lpthread #lpthread for handling threads
LDFLAGS_CLIENT :=

#Main dir
SRC_DIR     := ./src
BIN_DIR     := ./bin
HEADERS_DIR := ./headers

#Server structure
SERVER_SRC_DIR  := $(SRC_DIR)/server
SERVER_OBJ_DIR  := $(BIN_DIR)/server
#Find all .cpp files in src/server
SERVER_SRCS     := $(wildcard $(SERVER_SRC_DIR)/*.cpp)
#Map the .cpp files to .o files in bin/server
SERVER_OBJS     := $(patsubst $(SERVER_SRC_DIR)/%.cpp,$(SERVER_OBJ_DIR)/%.o,$(SERVER_SRCS))
SERVER_EXEC     := server.out

#Client structure
CLIENT_SRC_DIR  := $(SRC_DIR)/client
CLIENT_OBJ_DIR  := $(BIN_DIR)/client
#Find all .cpp files in src/client
CLIENT_SRCS     := $(wildcard $(CLIENT_SRC_DIR)/*.cpp)
#Map the .cpp files to .o files in bin/client
CLIENT_OBJS     := $(patsubst $(CLIENT_SRC_DIR)/%.cpp,$(CLIENT_OBJ_DIR)/%.o,$(CLIENT_SRCS))
CLIENT_EXEC     := client.out

.PHONY: all server client clean run-server run-client

#Compile both
all: $(SERVER_EXEC) $(CLIENT_EXEC)

#Compile only the server
server: $(SERVER_EXEC)

#Compile only the client
client: $(CLIENT_EXEC)

#Link .o files to server exec 
$(SERVER_EXEC): $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS_SERVER)

#Link .o files to client exec
$(CLIENT_EXEC): $(CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS_CLIENT)

#Compile .cpp server files to the .o files
$(SERVER_OBJ_DIR)/%.o: $(SERVER_SRC_DIR)/%.cpp | $(SERVER_OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

#Compile .cpp client files to the .o files
$(CLIENT_OBJ_DIR)/%.o: $(CLIENT_SRC_DIR)/%.cpp | $(CLIENT_OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

#Create bin dir
$(SERVER_OBJ_DIR) $(CLIENT_OBJ_DIR):
	mkdir -p $@

clean:
	rm -f $(SERVER_EXEC) $(CLIENT_EXEC)
	rm -rf $(BIN_DIR)

run-server: server
	./$(SERVER_EXEC)

run-client: client
	./$(CLIENT_EXEC)