#define _POSIX_C_SOURCE 200809L

#include "sig_handler.h"

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
