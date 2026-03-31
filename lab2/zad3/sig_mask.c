#define _POSIX_C_SOURCE 200809L

#include "sig_handler.h"

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
