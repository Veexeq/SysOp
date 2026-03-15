# Nazwa pliku wynikowego
TARGET = app

# Kompilator i flagi
CC = gcc
# -Wall -Wextra: włącza ostrzeżenia, -g: dodaje symbole debugowania
CFLAGS = -Wall -Wextra -g

# Znajdź automatycznie wszystkie pliki .c w obecnym katalogu
SRCS = $(wildcard *.c)
# Zamień rozszerzenia z .c na .o (pliki obiektowe)
OBJS = $(SRCS:.c=.o)

# Domyślny cel wywoływany po wpisaniu samego "make"
all: $(TARGET)

# Linkowanie (łączenie plików .o w plik wykonywalny)
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Reguła kompilacji każdego pliku .c do pliku .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Cel do czyszczenia projektu (usuwa pliki .o i plik wykonywalny)
clean:
	rm -f $(OBJS) $(TARGET)

# Oznaczenie celów, które nie są plikami
.PHONY: all clean
