#include "common.h"

int main(void) {
    // 1. Setup Shared Memory
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) { 
        perror("shm_open failed"); 
        exit(EXIT_FAILURE); 
    }

    if (ftruncate(shm_fd, sizeof(SharedMemory)) == -1) { 
        perror("ftruncate"); 
        exit(EXIT_FAILURE); 
    }

    SharedMemory *shm_ptr = mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    // 2. Setup Semaphores
    sem_t *mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);
    sem_t *empty = sem_open(SEM_EMPTY, O_CREAT, 0666, K);
    sem_t *full  = sem_open(SEM_FULL,  O_CREAT, 0666, 0);

    printf("[Consumer PID: %d] Started.\n", getpid());

    // 3. Main Loop
    for (int i = 0; i < 15; i++) {
        char task_str[TASK_LEN];
        const char *queue_source;

        sem_wait(full);
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

        sem_post(mutex);
        sem_post(empty);

        // Process outside critical section
        printf("\n[Consumer PID: %d] Processing %s task: ", getpid(), queue_source);
        for (int j = 0; task_str[j] != '\0'; j++) {
            putchar(task_str[j]);
            fflush(stdout);
            usleep(300000); 
        }
        printf(" (Done)\n");
    }

    // 4. Cleanup
    sem_close(mutex); sem_close(empty); sem_close(full);
    munmap(shm_ptr, sizeof(SharedMemory)); close(shm_fd);
    printf("[Consumer PID: %d] Finished.\n", getpid());
    return 0;
}
