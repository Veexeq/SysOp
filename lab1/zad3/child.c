#include "definitions.h"
#include <string.h>
#include <sys/file.h>

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

    FILE *file = fopen(OUTPUT_NAME, "a");
    if (!file) {
        perror("ERROR: couldn't open file\n");
        return 1;
    }
    int fd = fileno(file);

    char buf[32];
    snprintf(buf, sizeof(buf), "Potomek (%d)\n", getpid());
    size_t msg_length = strlen(buf);

    // Setting a lock. Exclusive lock works in such way, that processes
    // that encounter this lock will be put to sleep until the lock is released
    if (flock(fd, LOCK_EX) == -1) {
        perror("ERROR: Nie udało się założyć blokady\n");
        fclose(file);
        return 1;
    }

    for (int i = 0; i < M; ++i) {
        fwrite(buf, sizeof(char), msg_length, file);
        fflush(file);
    }

    // Releasing a lock
    if (flock(fd, LOCK_UN) == -1) {
        perror("ERROR: Nie udało się zdjąć blokady\n");
        fclose(file);
        return 1;
    }   

    fclose(file);

    return 0;
}