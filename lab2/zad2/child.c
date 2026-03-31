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

// Bezpieczna zmienna globalna dla zapisu otrzymanego trybu przetwarzania sygnału
volatile sig_atomic_t received_mode = 0;

void initialize_usr2_handling(void);
void usr2_handler(int, siginfo_t *, void *);
void mode_setup(char);
void sig_default(int);
void sig_mask(int);
void sig_unblock(int);
void sig_ignore(int);
void sig_handle(int);
void custom_handler(int);

int main(void) {
    initialize_usr2_handling();

    // Czekamy na otrzymanie sygnału
    while (received_mode == 0) {
        pause();
    }

    // Po zostaniu wybudzonym przez SIGUSR2, w received_mode znajduje się już 
    // informacja o trybie przetwarzania SIGUSR1
    mode_setup((char) received_mode);

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

void initialize_usr2_handling(void) {
    struct sigaction sa;
    sa.sa_sigaction = usr2_handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGUSR2, &sa, NULL) == -1) {
        perror("sigaction couldn't set signal handling for SIGUSR2");
        exit(EXIT_FAILURE);
    }
}

/**
 * Poprzednia wersja zakładała, że w tym miejscu (w handlerze, po otrzymaniu
 * sygnału) ustawiamy sposób, w jaki radzimy sobie z SIGUSR1 (ustawiamy maskę).
 * 
 * Problem z takim podejściem polega na tym, że jądro, wchodząc do handlera
 * wchodzi w stan awaryjny - przed wstąpieniem do niego robi snapshot, w którym
 * zawarta jest stara maska, bez zablokowanego SIGUSR1. W środku tego stanu
 * ustawiamy nową maskę, która później jest nadpisywana przez starą maskę, czyli
 * stan sprzed wejścia w handler. 
 * 
 * Należy ustawiać maskę zawsze z 'normalnego' trybu pracy jądra, nie z 
 * wnętrza handlera jakiegoś sygnału. 
 * 
 * Teraz nasz handler jedynie zapisuje do zmiennej globalnej jaki tryb wybrano.
 */
void usr2_handler(int signo, siginfo_t *info, void *context) {
    received_mode = info->si_value.sival_int;
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
        char msg[] = "improper mode passed into mode_setup\n";
        write(STDOUT_FILENO, msg, sizeof(msg) / sizeof(char) - 1);
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
