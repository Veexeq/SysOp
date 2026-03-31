# ====================================================================
# UŻYCIE
# 1. make clean   - usuwa wszystkie stare, skompilowane pliki
# 2. make static  - generuje libsig_handler.a oraz child_static.out
# 3. make shared  - generuje libsig_handler.so oraz child_shared.out
# 4. make dynamic - generuje libsig_handler.so oraz child_shared.out
# ====================================================================

# ====================================================================
# ZMIENNE
# ====================================================================
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L
# Lista plików biblioteki
LIB_SOURCES = sig_default.c sig_ignore.c sig_mask.c sig_handle.c
# Zamiana końcówek .c na .o
LIB_OBJECTS = $(LIB_SOURCES:.c=.o)

# Nazwy dla naszej biblioteki
LIB_NAME = sig_handler
LIB_STATIC = lib$(LIB_NAME).a
LIB_SHARED = lib$(LIB_NAME).so

# ====================================================================
# GŁÓWNA REGUŁA (Domyślnie buduje wszystko)
# ====================================================================
all: static shared dynamic

# ====================================================================
# REGUŁY BUDOWANIA BIBLIOTEK
# ====================================================================

# Reguła tworząca pliki .o (zwykłe)
%.o: %.c sig_handler.h
	$(CC) $(CFLAGS) -c $< -o $@

# Reguła tworząca bibliotekę statyczną (.a)
$(LIB_STATIC): $(LIB_OBJECTS)
	ar rcs $@ $^

# Reguła tworząca bibliotekę współdzieloną (.so)
# Uwaga: dla biblioteki współdzielonej musimy przebudować pliki .o z flagą -fPIC
$(LIB_SHARED): $(LIB_SOURCES) sig_handler.h
	$(CC) $(CFLAGS) -fPIC -c $(LIB_SOURCES)
	$(CC) -shared -Wl,-soname,$@ -o $@ $(LIB_OBJECTS)

# ====================================================================
# REGUŁY KOMPILACJI PROJEKTU (Warianty)
# ====================================================================

# 1. Wersja Statyczna
static: main.c child.c $(LIB_STATIC)
	$(CC) $(CFLAGS) main.c -o main
	$(CC) $(CFLAGS) child.c -L. -l$(LIB_NAME) -o child_static.out

# 2. Wersja Współdzielona (Shared)
# Używamy -Wl,-rpath,'$$ORIGIN', aby program wiedział, gdzie szukać pliku .so
shared: main.c child.c $(LIB_SHARED)
	$(CC) $(CFLAGS) main.c -o main
	$(CC) $(CFLAGS) child.c -L. -l$(LIB_NAME) -Wl,-rpath,'$$ORIGIN' -o child_shared.out

# 3. Wersja Ładowana Dynamicznie (Run-time / dlopen)
dynamic: main.c child.c $(LIB_SHARED)
	$(CC) $(CFLAGS) main.c -o main
	$(CC) $(CFLAGS) -DUSE_DYNAMIC child.c -ldl -o child_dynamic.out

# ====================================================================
# CZYSZCZENIE PROJEKTU
# ====================================================================
clean:
	rm -f *.o *.a *.so main child_static child_shared child_dynamic
