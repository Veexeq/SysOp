#include <stdio.h>
#include <signal.h>

void obsluga(int signum) {
    printf("Obsluga sygnalu\n");
}

int main(void) {
    signal(SIGUSR1, obsluga);
    raise(SIGUSR1);
    while(1);
    return 0;
}