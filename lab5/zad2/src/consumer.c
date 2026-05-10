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
        perror("ftruncate"); 
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

    printf("[Consumer PID: %d] Started.\n", getpid());

    // ==========================================
    // 3. MAIN CONSUMER LOOP
    // ==========================================

    while (1) {
        char task_str[TASK_LEN];
        const char *queue_source;

        // --- SYNCHRONIZATION START ---
        
        // 1. Decrease the count of full slots.
        // If full == 0, the consumer will BLOCK here until a producer inserts an item.
        sem_wait(full);

        // 2. Lock the shared memory
        sem_wait(mutex);

        // --- CRITICAL SECTION START ---

        // Consumers ALWAYS handle PRIORITY tasks first
        if (shm_ptr->priority_count > 0) {
            strncpy(task_str, shm_ptr->priority_queue[shm_ptr->priority_head].text, TASK_LEN);
            shm_ptr->priority_head = (shm_ptr->priority_head + 1) % K;
            shm_ptr->priority_count--;
            queue_source = "PRIORITY";
        } else {
            // If priority is empty, it must be in normal
            strncpy(task_str, shm_ptr->normal_queue[shm_ptr->normal_head].text, TASK_LEN);
            shm_ptr->normal_head = (shm_ptr->normal_head + 1) % K;
            shm_ptr->normal_count--;
            queue_source = "NORMAL";
        }
        
        // --- CRITICAL SECTION END ---

        // 3. Unlock the shared memory
        sem_post(mutex);

        // 4. Increase the count of empty slots.
        // This will wake up a blocked producer.
        sem_post(empty);

        // ==========================================
        // 4. PROCESS THE TASK (OUTSIDE CRITICAL SECTION)
        // ==========================================

        // It is crucial to process the data OUTSIDE the critical section.
        // If we did this while holding the mutex, we would block the entire system 
        // for several seconds just to print characters.

        printf("\n[Consumer PID: %d] Processing %s task: ", getpid(), queue_source);
        for (int j = 0; task_str[j] != '\0'; j++) {
            putchar(task_str[j]);
            fflush(stdout); // Force the terminal to print the character immediately
            usleep(300000); // 300,000 microseconds = 0.3 seconds
        }
        printf(" (Done)\n");
    }

    // ==========================================
    // 5. CLEANUP (LOCAL)
    // ==========================================

    sem_close(mutex); 
    sem_close(empty); 
    sem_close(full);
    
    munmap(shm_ptr, sizeof(SharedMemory)); close(shm_fd);
    printf("[Consumer PID: %d] Finished.\n", getpid());
    
    return EXIT_SUCCESS;
}
