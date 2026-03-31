/**
 * To makro "odblokowuje" w pliku nagłówkowym <signal.h> definicje struktur i funkcji, 
 * które nie należą do czystego języka C, ale są częścią standardu systemów Unix/Linux.
 */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

char parse_input(int, char *[]);

int main(int argc, char *argv[]) {
    char mode = parse_input(argc, argv);
    union sigval value;
    value.sival_int = (int) mode;

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork didn't work\n");
        _exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        execl("./child.out", "./child.out", NULL);

        perror("execl on ./child failed\n");
        _exit(EXIT_FAILURE);
    } else {
        sleep(1);

        if (sigqueue(pid, SIGUSR2, value) == -1) {
            perror("sigqueue couldn't send signal\n");
            kill(pid, SIGKILL);
            _exit(EXIT_FAILURE);
        }
    }

    wait(NULL);
}

char parse_input(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Invalid number of arguments. Expected: %s <default|mask|ignore|handle>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *input = argv[1];
    if (strcmp(input, "default") == 0) {
        return 'd';
    }
    if (strcmp(input, "mask") == 0) {
        return 'm';
    }
    if (strcmp(input, "ignore") == 0) {
        return 'i';
    }
    if (strcmp(input, "handle") == 0) {
        return 'h';
    }

    fprintf(stderr, "Invalid mode selected. Expected: %s <default|mask|ignore|handle>\n", argv[0]);
    exit(EXIT_FAILURE);
}
