# Kompilator i flagi
CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lm

# Ścieżki do folderów
Z1_DIR = zad1
Z2_DIR = zad2

# Pliki wykonywalne
EXEC_Z1 = $(Z1_DIR)/main.out
EXEC_Z2_P1 = $(Z2_DIR)/p1.out
EXEC_Z2_P2 = $(Z2_DIR)/p2.out

# Wszystkie cele
all: $(EXEC_Z1) $(EXEC_Z2_P1) $(EXEC_Z2_P2)
	@echo "Kompilacja zakończona sukcesem. Usuwanie plików obiektowych..."
	rm -f $(Z1_DIR)/*.o $(Z2_DIR)/*.o

# Zasada budowania Zadania 1
$(EXEC_Z1): $(Z1_DIR)/main.o $(Z1_DIR)/integrator.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Zasada budowania Zadania 2 - Program 1 (Klient)
$(EXEC_Z2_P1): $(Z2_DIR)/program1.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Zasada budowania Zadania 2 - Program 2 (Serwer)
$(EXEC_Z2_P2): $(Z2_DIR)/program2.o $(Z1_DIR)/integrator.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Uniwersalna zasada kompilacji plików .c do .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Czyszczenie plików wykonywalnych i ewentualnych pozostałości
clean:
	@echo "Sprzątanie plików wykonywalnych i obiektowych..."
	rm -f $(EXEC_Z1) $(EXEC_Z2_P1) $(EXEC_Z2_P2) $(Z1_DIR)/*.o $(Z2_DIR)/*.o
