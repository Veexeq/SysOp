#include "definitions.h"

int main(int argc, char *argv[]) {

    // Parse input
    if (argc != 2) {
        perror("ERROR: Nieprawidłowa liczba argumentów startowych.\n");
        printf("Użycie programu: %s <liczba>\n", argv[0]);
        return 1;
    }
    char *endptr;
    const long M = strtol(argv[1], &endptr, 10);

    if (endptr == argv[1] || *endptr != '\0') {
        perror("ERROR: Nieprawidłowy input.\n");
        printf("Użycie programu: %s <liczba>\n", argv[0]);
    }

    for (int i = 0; i < M; ++i) {
        printf("Potomek (%d)\n", getpid());
    }

    return 0;
}