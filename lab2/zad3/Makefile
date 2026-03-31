# ====================================================================
# UŻYCIE
# 1. make clean   - usuwa wszystkie stare, skompilowane pliki
# 2. make static  - generuje libsig_handler.a oraz child_static.out
# 3. make shared  - generuje libsig_handler.so oraz child_shared.out
# 4. make dynamic - generuje libsig_handler.so oraz child_shared.out
# ====================================================================

# ==========================================
# ZMIENNE
# ==========================================
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L
LIB_SOURCES = sig_default.c sig_ignore.c sig_mask.c sig_handle.c
LIB_OBJECTS = $(LIB_SOURCES:.c=.o)

LIB_NAME = sig_handler
LIB_STATIC = lib$(LIB_NAME).a
LIB_SHARED = lib$(LIB_NAME).so

# ==========================================
# GŁÓWNA REGUŁA
# ==========================================
all: static shared dynamic

# ==========================================
# REGUŁY BUDOWANIA BIBLIOTEK
# ==========================================

%.o: %.c sig_handler.h
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB_STATIC): $(LIB_OBJECTS)
	ar rcs $@ $^

$(LIB_SHARED): $(LIB_SOURCES) sig_handler.h
	$(CC) $(CFLAGS) -fPIC -c $(LIB_SOURCES)
	$(CC) -shared -Wl,-soname,$@ -o $@ $(LIB_OBJECTS)

# ==========================================
# REGUŁY KOMPILACJI PROJEKTU (Z Automatyzacją)
# ==========================================

static: main.c child.c $(LIB_STATIC)
	$(CC) $(CFLAGS) -DCHILD_PATH=\"./child_static.out\" main.c -o main.out
	$(CC) $(CFLAGS) child.c -L. -l$(LIB_NAME) -o child_static.out

shared: main.c child.c $(LIB_SHARED)
	$(CC) $(CFLAGS) -DCHILD_PATH=\"./child_shared.out\" main.c -o main.out
	$(CC) $(CFLAGS) child.c -L. -l$(LIB_NAME) -Wl,-rpath,'$$ORIGIN' -o child_shared.out

dynamic: main.c child.c $(LIB_SHARED)
	$(CC) $(CFLAGS) -DCHILD_PATH=\"./child_dynamic.out\" main.c -o main.out
	$(CC) $(CFLAGS) -DUSE_DYNAMIC child.c -ldl -o child_dynamic.out

# ==========================================
# CZYSZCZENIE PROJEKTU
# ==========================================
clean:
	rm -f *.o *.a *.so *.out
	