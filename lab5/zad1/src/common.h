#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>      // Flags O_*
#include <sys/stat.h>   // Access privilages definitions
#include <sys/mman.h>   // Functions regarding shared memory (shm_open, mmap)
#include <semaphore.h>  // Functions regarding POSIX semaphores
#include <time.h>
#include <errno.h>

#define K 5              // Buffer size
#define TASK_LEN 11      // 10 chars + '\0'

#define SHM_NAME "/lab5_z1_shm"
#define SEM_MUTEX "/lab5_z1_mutex"
#define SEM_EMPTY "/lab5_z1_empty"
#define SEM_FULL "/lab5_z1_full"

typedef struct _Task {
    char text[TASK_LEN];
} Task;

// Struct for a classic cyclic buffer
typedef struct _SharedMemory {
    Task buffer[K]; // Task array
    int head;       // Read index (for a consumer)
    int tail;       // Write index (for a producer)
} SharedMemory;

static inline void generate_random_string(char *str, int length) {
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    for (int i = 0; i < length; i++) {
        int key = rand() % (int)(sizeof(charset) - 1);
        str[i] = charset[key];
    }
    str[length] = '\0';
}

#endif // COMMON_H
