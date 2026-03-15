#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>

#define M 3

int zmiennaGlobalna = 1;

int main(int argc, char* argv[]) {

    // Parse input
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

    // Create `N` children
    for (int i = 0; i < N; ++i) {
        // `vfork()` works differently than `fork()`
        // 1. It creates a child process which shares memory with its parent
        // 2. It puts the parent process to sleep until its children dies (!)
        // Therefore, the output is printed sequentially, and the global var is incremented here.
        pid_t child_pid = vfork();
        
        // Child's path
        if (child_pid == 0) {
            zmiennaGlobalna++;
            for (int j = 0; j < M; ++j) {
                printf("Potomek (%d)\n", getpid());
                usleep(250000);
            }
            _exit(0);
        }
    }

    // Only the parent's process has access to this code, because
    // children have exited earlier. Wait for all child processes to exit here.
    for (int i = 0; i < N; ++i) {
        wait(NULL);
    }

    printf("Rodzic (%d) zmiennaGlobalna=%d\n", getpid(), zmiennaGlobalna);

    return 0;
}