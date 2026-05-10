#include "common.h"

int main(void) {
    // ==========================================
    // 1. SETUP SHARED MEMORY
    // ==========================================
    
    // Open the shared memory object. 
    // We use O_CREAT just in case the consumer is started before the producer.
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
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
    SharedMemory *shm_ptr = mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }

    // ==========================================
    // 2. SETUP SEMAPHORES
    // ==========================================
    
    // Open the semaphores.
    sem_t *mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);
    sem_t *empty = sem_open(SEM_EMPTY, O_CREAT, 0666, K);
    sem_t *full  = sem_open(SEM_FULL,  O_CREAT, 0666, 0);

    if (mutex == SEM_FAILED || empty == SEM_FAILED || full == SEM_FAILED) {
        perror("sem_open failed");
        exit(EXIT_FAILURE);
    }

    printf("[Consumer PID: %d] Started successfully. Waiting for tasks...\n", getpid());

    // ==========================================
    // 3. MAIN CONSUMER LOOP
    // ==========================================
    
    // Consume 10 tasks for demonstration purposes
    for (int i = 0; i < 10; i++) {
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
