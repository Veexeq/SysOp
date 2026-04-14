#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include "common.h"
#include "../zad1/integrator.h" 

// --- SIGNAL HANDLER ---
// Ta funkcja wykona się, gdy proces dostanie SIGINT lub SIGTERM
void handle_shutdown(int sig) {
    printf("\n[Program 2] Otrzymano sygnal %d. Zamykam serwer i sprzatam...\n", sig);
    
    // Usuwamy pliki FIFO z dysku
    if (unlink(FIFO_REQ) == -1) {
        perror("Blad podczas usuwania FIFO_REQ\n");
    } else {
        printf("Usunieto %s\n", FIFO_REQ);
    }
    
    if (unlink(FIFO_RES) == -1) {
        perror("Blad podczas usuwania FIFO_RES\n");
    } else {
        printf("Usunieto %s\n", FIFO_RES);
    }

    exit(EXIT_SUCCESS); // Czyste zamknięcie programu
}

int main() {
    printf("--- PROGRAM 2 (Liczący) ---\n");

    // --- REJESTRACJA OBSŁUGI SYGNAŁÓW ---
    struct sigaction sa;
    sa.sa_handler = handle_shutdown; // Wskazujemy naszą funkcję sprzątającą
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask); // Pusty koszyk blokowanych sygnałów
    
    // Podpinamy handler pod Ctrl+C (SIGINT)
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Blad rejestracji SIGINT\n");
        return EXIT_FAILURE;
    }
    
    // Podpinamy handler pod komendę kill (SIGTERM)
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("Blad rejestracji SIGTERM\n");
        return EXIT_FAILURE;
    }

    // Tworzenie potoków na dysku
    if (mkfifo(FIFO_REQ, 0666) == -1 && errno != EEXIST) {
        perror("Blad tworzenia FIFO_REQ\n");
        return EXIT_FAILURE;
    }
    if (mkfifo(FIFO_RES, 0666) == -1 && errno != EEXIST) {
        perror("Blad tworzenia FIFO_RES\n");
        return EXIT_FAILURE;
    }

    printf("[Program 2] Potoki gotowe. Wchodze w tryb nasluchiwania...\n");

    // Nieskończona pętla - obsłuży wiele zleceń po kolei
    while (1) {
        printf("\n[Program 2] Czekam na zlecenia w FIFO_REQ...\n");
        
        // 1. Otwieramy potok TYLKO do odczytu
        // Program zasypia w tym miejscu, aż Program 1 wpisze dane
        int fd_req = open(FIFO_REQ, O_RDONLY);
        if (fd_req == -1) {
            perror("Blad otwarcia FIFO_REQ\n");
            continue; // Zamiast ubijać proces, próbujemy ponownie w następnej pętli
        }

        // 2. Odbieramy dane
        Interval req;
        if (read(fd_req, &req, sizeof(Interval)) == sizeof(Interval)) {
            printf("[Program 2] Otrzymano zlecenie: [%.2f, %.2f], dx = %.9f\n", 
                   req.start, req.end, req.dx);
            
            // Zamykamy potok dla tego zlecenia
            close(fd_req);

            // 3. Rozpoczynamy obliczenia
            printf("[Program 2] Trwaja obliczenia...\n");
            double wynik = rectangular_integration(req.start, req.end, req.dx);
            printf("[Program 2] Obliczenia zakonczone (wynik: %.9f). Odsylam...\n", wynik);

            // 4. Otwieramy potok odpowiedzi tylko do zapisu
            int fd_res = open(FIFO_RES, O_WRONLY);
            if (fd_res != -1) {
                write(fd_res, &wynik, sizeof(double));
                close(fd_res);
                printf("[Program 2] Wynik pomyslnie wyslany.\n");
            } else {
                perror("Blad otwarcia FIFO_RES do odeslania wyniku\n");
            }
        } else {
            // Jeśli read zwróciło coś innego, to znaczy, że ktoś otworzył potok, 
            // ale nic nie wysłał (np. przerwał program 1 w trakcie działania)
            close(fd_req);
        }
    }

    return EXIT_SUCCESS;
}
