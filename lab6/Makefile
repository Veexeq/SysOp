CC = gcc
CFLAGS = -Wall -Wextra -pthread -g -I$(INCDIR)

INCDIR = include
SRCDIR = src
OBJDIR = obj
BINDIR = bin

$(shell mkdir -p $(OBJDIR) $(BINDIR))

.PHONY: all clean level1 level2 level3

# Default target alerts the user to build a specific level iteratively
all:
	@echo "Please specify a target to build: make level1, make level2, or make level3"

level1: $(BINDIR)/level1.out
level2: $(BINDIR)/level2.out
level3: $(BINDIR)/level3.out

# --- Object files compilation ---

$(OBJDIR)/common.o: $(SRCDIR)/common.c $(INCDIR)/common.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/common.c -o $(OBJDIR)/common.o

$(OBJDIR)/program1.o: $(SRCDIR)/program1.c $(INCDIR)/common.h
	$(CC) $(CFLAGS) -DLEVEL1 -c $(SRCDIR)/program1.c -o $(OBJDIR)/program1.o

$(OBJDIR)/program2.o: $(SRCDIR)/program2.c $(INCDIR)/common.h
	$(CC) $(CFLAGS) -DLEVEL2 -c $(SRCDIR)/program2.c -o $(OBJDIR)/program2.o

$(OBJDIR)/program3.o: $(SRCDIR)/program3.c $(INCDIR)/common.h
	$(CC) $(CFLAGS) -DLEVEL3 -c $(SRCDIR)/program3.c -o $(OBJDIR)/program3.o

# --- Linking executables ---

$(BINDIR)/level1.out: $(OBJDIR)/common.o $(OBJDIR)/program1.o
	$(CC) $(CFLAGS) $^ -o $@

$(BINDIR)/level2.out: $(OBJDIR)/common.o $(OBJDIR)/program2.o
	$(CC) $(CFLAGS) $^ -o $@

$(BINDIR)/level3.out: $(OBJDIR)/common.o $(OBJDIR)/program3.o
	$(CC) $(CFLAGS) $^ -o $@

# --- Cleanup ---
clean:
	rm -rf $(OBJDIR)/* $(BINDIR)/*
	rm -f *.jpg *.txt
	