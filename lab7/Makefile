# ==============================================================================
# Compiler Configuration
# ==============================================================================
CC = gcc
# -Wall -Wextra: Enable almost all compiler warnings
# -Werror: Treat all warnings as errors (strict code quality control)
# -g: Generate debugging symbols for GDB and Valgrind
# -std=c11: Use the ISO C11 standard
# -Iinclude: Look for header files inside the 'include' directory
CFLAGS = -Wall -Wextra -Werror -g -std=c11 -Iinclude

# ==============================================================================
# Directory Structure
# ==============================================================================
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# ==============================================================================
# Targets (Executables)
# ==============================================================================
SERVER_TARGET = $(BIN_DIR)/server
CLIENT_TARGET = $(BIN_DIR)/client

# ==============================================================================
# Source and Object Files Mapping
# ==============================================================================
SERVER_SRCS = $(SRC_DIR)/server/main.c
CLIENT_SRCS = $(SRC_DIR)/client/main.c

SERVER_OBJS = $(OBJ_DIR)/server/main.o
CLIENT_OBJS = $(OBJ_DIR)/client/main.o

# ==============================================================================
# Build Rules
# ==============================================================================
.PHONY: all clean

# Default rule to build both the server and the client
all: $(SERVER_TARGET) $(CLIENT_TARGET)

# Rule to link the server executable
$(SERVER_TARGET): $(SERVER_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(SERVER_OBJS) -o $(SERVER_TARGET)

# Rule to link the client executable
$(CLIENT_TARGET): $(CLIENT_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(CLIENT_OBJS) -o $(CLIENT_TARGET)

# Generic pattern rule to compile source files into object files
# Automatically creates the required subdirectories inside obj/
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to clean up all build artifacts
clean:
	@echo "Cleaning up build artifacts..."
	rm -rf $(OBJ_DIR) $(BIN_DIR)