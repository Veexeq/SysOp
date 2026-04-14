#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "common.h"

int main() {
    Interval req;

    printf("--- PROGRAM 1 (Zlecajacy) ---\n");
    printf("Podaj poczatek przedzialu: ");
    if (scanf("%lf", &req.start) != 1) return 1;
    
    printf("Podaj koniec przedzialu: ");
    if (scanf("%lf", &req.end) != 1) return 1;
    
    printf("Podaj dokladnosc (dx): ");
    if (scanf("%lf", &req.dx) != 1) return 1;

    printf("[Program 1] Czekam na otwarcie potoku FIFO_REQ...\n");

    // 1. Otwieramy potok tylko do zapisu (O_WRONLY)
    // UWAGA: Ten krok uśpi program, dopóki program2 nie wywoła open(O_RDONLY)
    int fd_req = open(FIFO_REQ, O_WRONLY);
    if (fd_req == -1) {
        perror("Blad otwarcia FIFO_REQ do zapisu\n");
        return EXIT_FAILURE;
    }

    // 2. Wysyłamy paczkę danych (całą strukturę naraz)
    printf("[Program 1] Wysylam zadanie...\n");
    if (write(fd_req, &req, sizeof(Interval)) != sizeof(Interval)) {
        perror("Blad zapisu struktury\n");
        close(fd_req);
        return EXIT_FAILURE;
    }
    
    // Zamykamy żądanie - wysłaliśmy co mieliśmy
    close(fd_req);

    // 3. Otwieramy potok odpowiedzi TYLKO do odczytu (O_RDONLY)
    // Znowu zostaniemy uśpieni, aż Program 2 skończy liczyć i otworzy tę rurę do zapisu
    printf("[Program 1] Zadanie wyslane. Czekam na wynik w FIFO_RES...\n");
    int fd_res = open(FIFO_RES, O_RDONLY);
    if (fd_res == -1) {
        perror("Blad otwarcia FIFO_RES do odczytu");
        return EXIT_FAILURE;
    }

    // 4. Czytamy wynik
    double wynik = 0.0;
    if (read(fd_res, &wynik, sizeof(double)) <= 0) {
        perror("Blad odczytu wyniku\n");
    } else {
        printf("\n>>> SUKCES! Odebrany wynik calki: %.9f <<<\n", wynik);
    }

    close(fd_res);
    return EXIT_SUCCESS;
}
