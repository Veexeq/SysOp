#include <stdio.h>
#include <signal.h>
#include <string.h>

void *get_signal_holder(const char *);

void sig_default();
void sig_mask();
void sig_ignore();
void sig_handle();

int main(int argc, char *argv[]) {

    if (argc != 2) {
        perror("ERROR: Nieprawidłowa liczba argumentów wywołania programu\n");
        printf("Niepoprawne wywołanie programu. Poprawnie: ./%s <default | mask | ignore | handle>\n", argv[1]);
        return 1;
    } 

    // signal(SIGUSR1, )

    for (int i = 0; i <= 20; ++i) {
        if (i == 5 || i == 15) {
            printf("Wysyłam sygnał USR1\n");
            raise(SIGUSR1);
        } else if (i == 10) {
            // TO-DO
        }
    }
    printf("Pętla została wywołana w całości.\n");

    return 0;
}

void *get_signal_handler(const char *mode) {
    if (strcmp(mode, "default") == 0) {
        return 
    }
}
