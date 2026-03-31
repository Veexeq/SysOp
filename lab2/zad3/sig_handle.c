#define _POSIX_C_SOURCE 200809L

#include "sig_handler.h"

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
