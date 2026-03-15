#include <stdio.h>
#include <stdlib.h>

const int zmiennaGlobalna = 1;

int main(int argc, char* argv[]) {

    // PARSE INPUT
    if (argc != 2) {
        perror("ERROR: Nieprawidłowa liczba argumentów startowych.\n");
        printf("Użycie programu: %s <liczba>\n", argv[0]);
        return 1;
    }
    char *endptr;
    const long N = strtol(argv[1], &endptr, 10);

    if (endptr == argv[1] || *endptr != '\0') {
        perror("ERROR: Nieprawidłowy input.\n");
        printf("Użycie programu: %s <liczba>\n", argv[0]);
    }

    

    return 0;
}