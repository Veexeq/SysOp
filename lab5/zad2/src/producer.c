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

    // ==========================================
    // 2. SETUP SEMAPHORES
    // ==========================================

    // Open (or create) the semaphores.
    // If they already exist, the initial values (1, K, 0) are safely ignored by the OS.
    sem_t *mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);
    sem_t *empty = sem_open(SEM_EMPTY, O_CREAT, 0666, K);
    sem_t *full  = sem_open(SEM_FULL,  O_CREAT, 0666, 0);

    printf("[Producer PID: %d] Started.\n", getpid());

    // ==========================================
    // 3. MAIN PRODUCER LOOP
    // ==========================================

    // Produce 15 tasks to see the 30% distribution
    for (int i = 0; i < 15; i++) { 
        char task_str[TASK_LEN];
        generate_random_string(task_str, TASK_LEN - 1);

        // Determine priority: 30% chance for PRIORITY
        bool is_priority = (rand() % 100) < 30;
        const char *queue_name = is_priority ? "PRIORITY" : "NORMAL";

        printf("[Producer PID: %d] Created %s task: '%s'. Waiting...\n", getpid(), queue_name, task_str);

        // --- SYNCHRONIZATION START ---

        // 1. Decrease the count of empty slots. 
        // If empty == 0, the producer will BLOCK here until a consumer reads an item.
        sem_wait(empty);

        // 2. Lock the shared memory to prevent race conditions
        sem_wait(mutex);

        // --- CRITICAL SECTION START ---

        // Copy the generated string into the buffer at the current 'tail' index
        // Choose the queue based on whether we produced a priority task, or not
        if (is_priority) {
            strncpy(shm_ptr->priority_queue[shm_ptr->priority_tail].text, task_str, TASK_LEN);
            shm_ptr->priority_tail = (shm_ptr->priority_tail + 1) % K;
            shm_ptr->priority_count++;
        } else {
            strncpy(shm_ptr->normal_queue[shm_ptr->normal_tail].text, task_str, TASK_LEN);
            shm_ptr->normal_tail = (shm_ptr->normal_tail + 1) % K;
            shm_ptr->normal_count++;
        }

        // --- CRITICAL SECTION END ---

        // 3. Unlock the shared memory
        sem_post(mutex);
        
        // 4. Increase the count of full slots.
        // This will wake up a consumer if one is currently blocked waiting for a task.
        sem_post(full);

        // --- SYNCHRONIZATION END ---

        // Sleep for a random amount of time (0 to 2 seconds) to simulate workload
        sleep(rand() % 2);
    }

    printf("[Producer PID: %d] Finished producing 15 tasks. Disconnecting...\n", getpid());

    // ==========================================
    // 4. CLEANUP (LOCAL)
    // ==========================================
    
    // We strictly CLOSE the semaphores and UNMAP the memory.
    // We DO NOT use sem_unlink() or shm_unlink() here, because other producers
    // or consumers might still be using the buffer!
    sem_close(mutex); 
    sem_close(empty); 
    sem_close(full);

    munmap(shm_ptr, sizeof(SharedMemory)); close(shm_fd);
    printf("[Producer PID: %d] Finished.\n", getpid());

    return EXIT_SUCCESS;
}
