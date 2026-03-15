#include "definitions.h"
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {

    // Parse input
    if (argc != 3) {
        perror("ERROR: Nieprawidłowa liczba argumentów startowych.\n");
        printf("Użycie programu: %s <liczba> <liczba>\n", argv[0]);
        return 1;
    }

    char *endptr_n;
    char *endptr_m;

    const long N = strtol(argv[1], &endptr_n, 10);
    const long M = strtol(argv[2], &endptr_m, 10);

    if (endptr_n == argv[1] || *endptr_n != '\0' ||
        endptr_m == argv[2] || *endptr_m != '\0') {
        perror("ERROR: Nieprawidłowy input.\n");
        printf("Użycie programu: %s <liczba> <liczba>\n", argv[0]);
    }

    // Remove output file
    int status;
    pid_t remover_pid = fork();
    if (remover_pid == 0) {
        execlp("rm", "rm", OUTPUT_NAME, NULL);
    } else {
        wait(&status);
    }

    if (status != 0) {
        perror("ERROR: removing file unsuccessful.\n");
        printf("Nie udało się usunąć pliku %s\n", OUTPUT_NAME);
        return 1;
    }

    for (int i = 0; i < N; ++i) {
        pid_t child_pid = fork();
        if (child_pid == 0) {
            execl("./child.out", "child.out", argv[2], NULL);
        }
    }

    for (int i = 0; i < N; ++i) {
        wait(NULL);
    }

    printf("Rodzic  (%d)\n", getpid());

    return 0;
}