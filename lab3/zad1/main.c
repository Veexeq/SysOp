#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include "integrator.h"

int main(int argc, char *argv[]) {
    
    if (argc != 3) {
        fprintf(stderr, "Użycie: %s <dx> <n>\n", argv[0]);
        return EXIT_FAILURE;
    }
    char *endptr;
    const double dx = strtod(argv[1], &endptr);
    if (endptr == argv[1] || *endptr != '\0' || dx <= 0.0) {
        fprintf(stderr, "Błąd: dx musi być dodatnią liczbą zmiennoprzecinkową.\n");
        return EXIT_FAILURE;
    }
    
    const long n = strtol(argv[2], &endptr, 10);
    if (endptr == argv[2] || *endptr != '\0' || n <= 0) {
        fprintf(stderr, "Błąd: n musi być dodatnią liczbą całkowitą.\n");
        return EXIT_FAILURE;
    }

    // Główna pętla programu - testujemy dla k = 1, 2, ..., n procesów
    for (int k = 1; k <= n; ++k) {
        
        // Struktury do mierzenia czasu
        struct timespec start_time, end_time;
        clock_gettime(CLOCK_MONOTONIC, &start_time);

        // Bezpieczna dynamiczna alokacja tablicy potoków (k potoków, każdy ma 2 końce)
        // int (*fd)[2] to wskaźnik na tablice 2-elementowe typu int
        int (*fd)[2] = malloc(k * sizeof(*fd));
        if (fd == NULL) {
            perror("Błąd alokacji pamięci malloc");
            return EXIT_FAILURE;
        }

        // Szerokość przedziału, który będzie liczył pojedynczy proces
        // Cały przedział to [0, 1], więc dzielimy go na k równych części
        double slice_width = 1.0 / k;

        // Tworzenie procesów potomnych
        for (int i = 0; i < k; ++i) {
            if (pipe(fd[i]) == -1) {
                perror("Błąd tworzenia potoku pipe");
                free(fd);
                return EXIT_FAILURE;
            }

            pid_t pid = fork();
            if (pid < 0) {
                perror("Błąd tworzenia procesu fork");
                free(fd);
                return EXIT_FAILURE;
            }

            if (pid == 0) { 
                // --- KOD PROCESU POTOMNEGO ---
                
                // Dziecko zamyka swój wlot do czytania (będzie tylko pisać)
                close(fd[i][0]);

                // BARDZO WAŻNE: Dziecko dziedziczy też otwarte wloty do czytania 
                // od swoich starszych braci (utworzonych w poprzednich iteracjach).
                // Musimy je bezwzględnie zamknąć.
                for (int j = 0; j < i; ++j) {
                    close(fd[j][0]);
                }

                // Obliczamy ramy dla tego konkretnego procesu
                double start_x = i * slice_width;
                double end_x = (i + 1) * slice_width;

                // Liczymy całkę dla zadanego fragmentu
                double partial_sum = rectangular_integration(start_x, end_x, dx);

                // Wysyłamy wynik do rodzica. write() jest bezpieczne dla małych danych (double = 8 bajtów)
                if (write(fd[i][1], &partial_sum, sizeof(double)) != sizeof(double)) {
                    perror("Błąd zapisu do potoku write");
                    exit(EXIT_FAILURE); // Używamy exit(), a nie return, żeby ubić dziecko!
                }
                
                // Sprzątamy i bezwarunkowo kończymy życie dziecka
                close(fd[i][1]);
                exit(EXIT_SUCCESS); 
            } 
            else { 
                // --- KOD RODZICA (wewnątrz pętli tworzącej) ---
                
                // Rodzic od razu zamyka wylot do pisania w nowym potoku. 
                // Chroni to przed deadlockiem.
                close(fd[i][1]);
            }
        }

        // --- KOD RODZICA (po stworzeniu wszystkich dzieci) ---
        double total_sum = 0.0;

        // Odbieranie wyników od każdego dziecka
        for (int i = 0; i < k; ++i) {
            double partial_sum = 0.0;
            
            // Czytamy wynik. Skoro zamknęliśmy fd[i][1], read poczeka na dane, 
            // a gdy dziecko wyśle i się zamknie, read() zwróci liczbę przeczytanych bajtów.
            if (read(fd[i][0], &partial_sum, sizeof(double)) > 0) {
                total_sum += partial_sum;
            }
            
            // Zamykamy odczyt po odebraniu danych
            close(fd[i][0]); 
            
            // Czekamy na definitywne zakończenie procesu dziecka (unikamy "zombie")
            wait(NULL); 
        }

        // Zatrzymanie stopera
        clock_gettime(CLOCK_MONOTONIC, &end_time);
        
        // Obliczenie różnicy czasu w sekundach
        double elapsed = (end_time.tv_sec - start_time.tv_sec) + 
                         (end_time.tv_nsec - start_time.tv_nsec) / 1e9;

        // Wypisanie wyniku
        printf("Liczba procesów (k): %3d | Wynik całki: %.9f | Czas wykonania: %.6f s\n", 
               k, total_sum, elapsed);

        // Zwalniamy pamięć zaalokowaną na potoki dla tej iteracji 'k'
        free(fd);
    }

    return EXIT_SUCCESS;
}
