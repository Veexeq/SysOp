#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <time.h>
#include <stdbool.h>
#include <errno.h>

#define K 10            // Max total capacity of the system
#define TASK_LEN 11     // 10 chars + '\0'

#define SHM_NAME "/lab5_z3_shm"
#define SEM_MUTEX "/lab5_z3_mutex"
#define SEM_EMPTY "/lab5_z3_empty"
#define SEM_FULL "/lab5_z3_full"

typedef struct {
    char text[TASK_LEN];
} Task;

// Shared Memory Structure with Two Queues
typedef struct {
    // Priority Queue
    Task priority_queue[K];
    int priority_head;
    int priority_tail;
    int priority_count;

    // Normal Queue
    Task normal_queue[K];
    int normal_head;
    int normal_tail;
    int normal_count;
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
