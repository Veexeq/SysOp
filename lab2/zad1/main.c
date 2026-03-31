/**
 * To makro "odblokowuje" w pliku nagłówkowym <signal.h> definicje struktur i funkcji, 
 * które nie należą do czystego języka C, ale są częścią standardu systemów Unix/Linux.
 */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

char parse_input(int, char *[]);
void mode_setup(char);
void sig_default(int);
void sig_mask(int);
void sig_unblock(int);
void sig_ignore(int);
void sig_handle(int);
void custom_handler(int);

int main(int argc, char *argv[]) {
    char mode = parse_input(argc, argv);
    mode_setup(mode);

    for (int i = 1; i <= 20; ++i) {
        printf("%d\n", i);

        if (i == 5 || i == 15) {
            printf("Wysyłam sygnał USR1\n");
            raise(SIGUSR1);
        } else if (i == 10) {
            sigset_t set;
            sigemptyset(&set);

            if (sigpending(&set) == 0) {
                if (sigismember(&set, SIGUSR1)) {
                    printf("Odblokowuję USR1\n");
                    sig_unblock(SIGUSR1);
                }
            } else {
                perror("sigpending couldn't check pending signals\n");
                exit(EXIT_FAILURE);
            }
        }
        
        sleep(1);
    }

    printf("Pętla została wykonana w całości\n");

    return 0;
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

void mode_setup(char mode) {
    int signo = SIGUSR1;

    switch (mode)
    {
    case 'd':
        sig_default(signo);
        break;
    case 'm':
        sig_mask(signo);
        break;
    case 'i':
        sig_ignore(signo);
        break;
    case 'h':
        sig_handle(signo);
        break;
    default:
        fprintf(stderr, "Invalid mode %c parsed in %s\n", mode, __func__);
        break;
    }
}

void sig_default(int signo) {
    struct sigaction sa;
    sa.sa_handler = SIG_DFL;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(signo, &sa, NULL) == -1) {
        perror("sigaction couldn't set signal handling\n");
        exit(EXIT_FAILURE);
    }
}

void sig_mask(int signo) {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, signo);

    // For this exercise, we don't need to keep information about the previous mask
    if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) {
        perror("sigprocmask couldn't set signal masking\n");
        exit(EXIT_FAILURE);
    }
}

void sig_unblock(int signo) {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, signo);

    if (sigprocmask(SIG_UNBLOCK, &mask, NULL) < 0) {
        perror("sigprocmask couldn't unblock signal masking\n");
        exit(EXIT_FAILURE);
    }
}

void sig_ignore(int signo) {
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(signo, &sa, NULL) == -1) {
        perror("sigaction couldn't set signal handling\n");
        exit(EXIT_FAILURE);
    }
}

void sig_handle(int signo) {
    struct sigaction sa;
    sa.sa_handler = custom_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(signo, &sa, NULL) == -1) {
        perror("sigaction couldn't set signal handling\n");
        exit(EXIT_FAILURE);
    }    
}

/**
 * Musimy użyć bardziej niskopoziomowego write zamiast printf, gdyż 
 * printf nie jest async-signal-safe.
 * Jego użycie może prowadzić do zakleszczenia.
 */
void custom_handler(int signo) {
    char msg[] = "Wywołano handler dla sygnału **\n";
    int len = sizeof(msg) / sizeof(char) - 1;

    // Uzupełniamy wiadomość o dwucyfrowy numer sygnału
    msg[len - 2] = '0' + signo % 10;
    msg[len - 3] = '0' + signo / 10;

    write(STDOUT_FILENO, msg, len);
}
