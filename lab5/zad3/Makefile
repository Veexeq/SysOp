# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11 -g
# Important: -lrt for SHM, -lpthread for semaphores
LDFLAGS = -lrt -lpthread

# Directory structure
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Executable names
PRODUCER_BIN = $(BIN_DIR)/producer.out
CONSUMER_BIN = $(BIN_DIR)/consumer.out
MANAGER_BIN  = $(BIN_DIR)/manager.out
CLEANER_BIN  = $(BIN_DIR)/cleaner.out

.PHONY: all clean directories producer consumer manager cleaner

# Default target
all: directories $(PRODUCER_BIN) $(CONSUMER_BIN) $(MANAGER_BIN) $(CLEANER_BIN)

# Friendly aliases
producer: directories $(PRODUCER_BIN)
consumer: directories $(CONSUMER_BIN)
manager:  directories $(MANAGER_BIN)
cleaner:  directories $(CLEANER_BIN)

# Directory creation
directories:
	@mkdir -p $(BIN_DIR) $(OBJ_DIR)

# Linking Phase
$(PRODUCER_BIN): $(OBJ_DIR)/producer.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(CONSUMER_BIN): $(OBJ_DIR)/consumer.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(MANAGER_BIN): $(OBJ_DIR)/manager.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(CLEANER_BIN): $(OBJ_DIR)/cleaner.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Compiling Phase (all depend on common.h)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

# Clean Phase
clean:
	@echo "Cleaning up build artifacts..."
	rm -rf $(BIN_DIR) $(OBJ_DIR)
	