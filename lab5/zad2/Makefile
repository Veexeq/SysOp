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
CLEANER_BIN  = $(BIN_DIR)/cleaner.out

.PHONY: all clean directories producer consumer cleaner

# Default target
all: directories $(PRODUCER_BIN) $(CONSUMER_BIN) $(CLEANER_BIN)

# Friendly aliases
producer: directories $(PRODUCER_BIN)
consumer: directories $(CONSUMER_BIN)
cleaner:  directories $(CLEANER_BIN)

# Directory creation
directories:
	@mkdir -p $(BIN_DIR) $(OBJ_DIR)

# Linking Producer
$(PRODUCER_BIN): $(OBJ_DIR)/producer.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Linking Consumer
$(CONSUMER_BIN): $(OBJ_DIR)/consumer.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Linking Cleaner
$(CLEANER_BIN): $(OBJ_DIR)/cleaner.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Compiling source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

# Cleaning up
clean:
	@echo "Cleaning up build artifacts..."
	rm -rf $(BIN_DIR) $(OBJ_DIR)
	