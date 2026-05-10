#include "common.h"

int main(void) {
    // Seed the random number generator using the current time and process ID
    srand(time(NULL) ^ getpid());

    // ==========================================
    // 1. SETUP SHARED MEMORY
    // ==========================================
    
    // Open (or create) the shared memory object
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        exit(EXIT_FAILURE);
    }

    // Set the size of the shared memory object.
    // Important: A newly created shared memory object has a size of 0.
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
    
    // Open (or create) the semaphores.
    // If they already exist, the initial values (1, K, 0) are safely ignored by the OS.
    sem_t *mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);
    sem_t *empty = sem_open(SEM_EMPTY, O_CREAT, 0666, K);
    sem_t *full  = sem_open(SEM_FULL,  O_CREAT, 0666, 0);

    if (mutex == SEM_FAILED || empty == SEM_FAILED || full == SEM_FAILED) {
        perror("sem_open failed");
        exit(EXIT_FAILURE);
    }

    printf("[Producer PID: %d] Started successfully.\n", getpid());

    // ==========================================
    // 3. MAIN PRODUCER LOOP
    // ==========================================
    
    // Produce 10 tasks for demonstration purposes
    for (int i = 0; i < 10; i++) {
        char task_str[TASK_LEN];
        generate_random_string(task_str, TASK_LEN - 1); // Generate 10 chars + '\0'

        printf("[Producer PID: %d] Generated task: '%s'. Waiting for space...\n", getpid(), task_str);

        // --- SYNCHRONIZATION START ---
        
        // 1. Decrease the count of empty slots. 
        // If empty == 0, the producer will BLOCK here until a consumer reads an item.
        sem_wait(empty);

        // 2. Lock the shared memory to prevent race conditions
        sem_wait(mutex);

        // --- CRITICAL SECTION START ---
        
        // Copy the generated string into the buffer at the current 'tail' index
        strncpy(shm_ptr->buffer[shm_ptr->tail].text, task_str, TASK_LEN);
        printf("[Producer PID: %d] Inserted '%s' at index %d\n", getpid(), task_str, shm_ptr->tail);

        // Move the tail index forward circularly
        shm_ptr->tail = (shm_ptr->tail + 1) % K;

        // --- CRITICAL SECTION END ---

        // 3. Unlock the shared memory
        sem_post(mutex);

        // 4. Increase the count of full slots.
        // This will wake up a consumer if one is currently blocked waiting for a task.
        sem_post(full);

        // --- SYNCHRONIZATION END ---

        // Sleep for a random amount of time (0 to 2 seconds) to simulate workload
        sleep(rand() % 3);
    }

    printf("[Producer PID: %d] Finished producing 10 tasks. Disconnecting...\n", getpid());

    // ==========================================
    // 4. CLEANUP (LOCAL)
    // ==========================================
    
    // We strictly CLOSE the semaphores and UNMAP the memory.
    // We DO NOT use sem_unlink() or shm_unlink() here, because other producers
    // or consumers might still be using the buffer!
    sem_close(mutex);
    sem_close(empty);
    sem_close(full);
    
    munmap(shm_ptr, sizeof(SharedMemory));
    close(shm_fd);

    return 0;
}
