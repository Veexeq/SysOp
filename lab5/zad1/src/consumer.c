#include "common.h"
#include <signal.h>

// Global IPC variables required for the signal handler to close them properly
int shm_fd = -1;
SharedMemory *shm_ptr = MAP_FAILED;
sem_t *mutex = SEM_FAILED;
sem_t *empty = SEM_FAILED;
sem_t *full = SEM_FAILED;

// Shutdown handler for SIGINT and SIGTERM
void cleanup_handler(int sig) {
    printf("\n[Consumer PID: %d] Received signal (%d). Cleaning up...\n", getpid(), sig);

    if (mutex != SEM_FAILED) sem_close(mutex);
    if (empty != SEM_FAILED) sem_close(empty);
    if (full != SEM_FAILED)  sem_close(full);
    
    if (shm_ptr != MAP_FAILED) munmap(shm_ptr, sizeof(SharedMemory));
    if (shm_fd != -1)          close(shm_fd);

    exit(EXIT_SUCCESS);
}

int main(void) {
    // Register the signal handler
    signal(SIGINT, cleanup_handler);
    signal(SIGTERM, cleanup_handler);

    // ==========================================
    // 1. SETUP SHARED MEMORY
    // ==========================================
    
    // Open the shared memory object. 
    // We use O_CREAT just in case the consumer is started before the producer.
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        exit(EXIT_FAILURE);
    }

    // Set the size (safeguard if consumer starts first)
    if (ftruncate(shm_fd, sizeof(SharedMemory)) == -1) {
        perror("ftruncate failed");
        exit(EXIT_FAILURE);
    }

    // Map the shared memory into the process's virtual address space
    shm_ptr = mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }

    // ==========================================
    // 2. SETUP SEMAPHORES
    // ==========================================
    
    // Open the semaphores.
    mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);
    empty = sem_open(SEM_EMPTY, O_CREAT, 0666, K);
    full  = sem_open(SEM_FULL,  O_CREAT, 0666, 0);

    if (mutex == SEM_FAILED || empty == SEM_FAILED || full == SEM_FAILED) {
        perror("sem_open failed");
        exit(EXIT_FAILURE);
    }

    printf("[Consumer PID: %d] Started successfully. Waiting for tasks...\n", getpid());

    // ==========================================
    // 3. MAIN CONSUMER LOOP
    // ==========================================
    
    while (1) {
        char task_str[TASK_LEN];

        // --- SYNCHRONIZATION START ---
        
        // 1. Decrease the count of full slots.
        // If full == 0, the consumer will BLOCK here until a producer inserts an item.
        sem_wait(full);

        // 2. Lock the shared memory
        sem_wait(mutex);

        // --- CRITICAL SECTION START ---
        
        // Copy the task from the buffer at the current 'head' index
        strncpy(task_str, shm_ptr->buffer[shm_ptr->head].text, TASK_LEN);
        
        // Move the head index forward circularly
        shm_ptr->head = (shm_ptr->head + 1) % K;

        // --- CRITICAL SECTION END ---

        // 3. Unlock the shared memory
        sem_post(mutex);

        // 4. Increase the count of empty slots.
        // This will wake up a blocked producer.
        sem_post(empty);

        // --- SYNCHRONIZATION END ---

        // ==========================================
        // 4. PROCESS THE TASK (OUTSIDE CRITICAL SECTION)
        // ==========================================

        // It is crucial to process the data OUTSIDE the critical section.
        // If we did this while holding the mutex, we would block the entire system 
        // for several seconds just to print characters.
        
        printf("\n[Consumer PID: %d] Processing task: ", getpid());
        
        // Print character by character with a 0.3s delay
        for (int j = 0; task_str[j] != '\0'; j++) {
            putchar(task_str[j]);
            fflush(stdout); // Force the terminal to print the character immediately
            usleep(300000); // 300,000 microseconds = 0.3 seconds
        }
        printf(" (Done)\n");
    }

    printf("[Consumer PID: %d] Finished consuming 10 tasks. Disconnecting...\n", getpid());

    // ==========================================
    // 5. CLEANUP (LOCAL)
    // ==========================================
    
    sem_close(mutex);
    sem_close(empty);
    sem_close(full);
    
    munmap(shm_ptr, sizeof(SharedMemory));
    close(shm_fd);

    return EXIT_SUCCESS;
}
