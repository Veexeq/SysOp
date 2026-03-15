#
# Minimal Makefile which compiles multiple C files into individual executables.
#
#
# - Sarah Mount, November 2011 (Modified for .out targets)
#

CC=gcc

RM=rm

CFLAGS=-c -Wall -O3

LDFLAGS=-lm

SOURCES=$(wildcard *.c)

OBJECTS=$(SOURCES:.c=.o)

# ZMIANA 1: Podmieniamy końcówkę .c na .out zamiast ucinać ją całkowicie
EXECS=$(SOURCES:.c=.out)

.PHONY: all
all: $(OBJECTS) $(EXECS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

# ZMIANA 2: Zastępujemy stare ".o.:" nowoczesną regułą.
# Mówi ona: "aby zbudować plik .out, potrzebujesz pliku .o o tej samej nazwie"
%.out: %.o
	$(CC) $^ $(LDFLAGS) -o $@

.PHONY: clean
clean:
	-@ $(RM) *.o 
	-@ $(RM) $(EXECS)