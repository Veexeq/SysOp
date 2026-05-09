# Kompilator i flagi
CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11 -g
LDFLAGS = -lrt

# Struktura katalogów
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Nazwy plików wykonywalnych
SERVER_BIN = $(BIN_DIR)/server.out
CLIENT_BIN = $(BIN_DIR)/client.out

# Cele sztuczne (dodane 'server' i 'client')
.PHONY: all clean rebuild directories server client

# Domyślny cel
all: directories $(SERVER_BIN) $(CLIENT_BIN)

# === PRZYJAZNE ALIASY ===
server: directories $(SERVER_BIN)
client: directories $(CLIENT_BIN)

# Tworzenie katalogów
directories:
	@mkdir -p $(BIN_DIR) $(OBJ_DIR)

# Kompilacja końcowa Serwera
$(SERVER_BIN): $(OBJ_DIR)/server.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Kompilacja końcowa Klienta
$(CLIENT_BIN): $(OBJ_DIR)/client.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Uniwersalna reguła kompilacji plików źródłowych (.c) do obiektowych (.o)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/common.h
	$(CC) $(CFLAGS) -c $< -o $@

# Dodatkowa zależność: server.o zależy również od server.h
$(OBJ_DIR)/server.o: $(SRC_DIR)/server.h

# Pełne czyszczenie
clean:
	@echo "Sprzątanie plików wykonywalnych i obiektowych..."
	rm -rf $(BIN_DIR) $(OBJ_DIR)

# Szybkie przebudowanie od zera
rebuild: clean all
