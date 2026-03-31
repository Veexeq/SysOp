/**
 * To makro "odblokowuje" w pliku nagłówkowym <signal.h> definicje struktur i funkcji, 
 * które nie należą do czystego języka C, ale są częścią standardu systemów Unix/Linux.
 */
#define _POSIX_C_SOURCE 200809L

#ifdef USE_DYNAMIC
    #include <dlfcn.h>
#endif

#include "sig_handler.h"
#include <string.h>

// Bezpieczna zmienna globalna dla zapisu otrzymanego trybu przetwarzania sygnału
volatile sig_atomic_t received_mode = 0;

void initialize_usr2_handling(void);
void usr2_handler(int, siginfo_t *, void *);
void mode_setup(char);
void sig_unblock(int);

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

    #ifdef USE_DYNAMIC
        void *handle;
        void (*handler_func)(int);
        char *error;
        const char *func_name = NULL;

        switch (mode) {
            case 'd': 
                func_name = "sig_default"; 
                break;
            case 'm': 
                func_name = "sig_mask"; 
                break;
            case 'i': 
                func_name = "sig_ignore"; 
                break;
            case 'h': 
                func_name = "sig_handle"; 
                break;
            default: {
                char msg[] = "improper mode passed into mode_setup\n";
                write(STDOUT_FILENO, msg, sizeof(msg) - 1);
                exit(EXIT_FAILURE);
            }
        }

        handle = dlopen("./libsig_handler.so", RTLD_LAZY);
        if (!handle) {
            fprintf(stderr, "dlopen error: %s\n", dlerror());
            exit(EXIT_FAILURE);
        }

        // Robimy 'dump' starych błędów w próżnię
        dlerror();

        handler_func = (void (*)(int)) dlsym(handle, func_name);
        if ((error = dlerror()) != NULL) {
            fprintf(stderr, "dlsym error: %s\n", error);
            dlclose(handle);
            exit(EXIT_FAILURE);
        }

        // Wywołujemy funkcję z libsig_handler.so
        handler_func(signo);

        dlclose(handle);
    #else
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
    #endif
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
