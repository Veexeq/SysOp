#include "common.h"

int main(void) {
    srand(time(NULL) ^ getpid());

    // 1. Setup Shared Memory
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) { 
        perror("shm_open failed"); 
        exit(EXIT_FAILURE); 
    }

    if (ftruncate(shm_fd, sizeof(SharedMemory)) == -1) { 
        perror("ftruncate failed"); 
        exit(EXIT_FAILURE); 
    }

    SharedMemory *shm_ptr = mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    // 2. Setup Semaphores
    sem_t *mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);
    sem_t *empty = sem_open(SEM_EMPTY, O_CREAT, 0666, K);
    sem_t *full  = sem_open(SEM_FULL,  O_CREAT, 0666, 0);

    printf("[Producer PID: %d] Started.\n", getpid());

    // 3. Main Loop
    for (int i = 0; i < 15; i++) { 
        // Produce 15 tasks to see the 30% distribution
        char task_str[TASK_LEN];
        generate_random_string(task_str, TASK_LEN - 1);

        // Determine priority: 30% chance for PRIORITY
        bool is_priority = (rand() % 100) < 30;
        const char *queue_name = is_priority ? "PRIORITY" : "NORMAL";

        printf("[Producer PID: %d] Created %s task: '%s'. Waiting...\n", getpid(), queue_name, task_str);

        sem_wait(empty);
        sem_wait(mutex);

        // --- CRITICAL SECTION START ---
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

        sem_post(mutex);
        sem_post(full);

        sleep(rand() % 2); // Workload simulation
    }

    // 4. Cleanup
    sem_close(mutex); sem_close(empty); sem_close(full);
    munmap(shm_ptr, sizeof(SharedMemory)); close(shm_fd);
    printf("[Producer PID: %d] Finished.\n", getpid());
    return 0;
}
