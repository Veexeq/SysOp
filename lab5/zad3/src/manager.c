#include "common.h"
#include <signal.h>

// Global variables for IPC objects
int shm_fd = -1;
SharedMemory *shm_ptr = MAP_FAILED;
sem_t *mutex = SEM_FAILED;

// We open empty and full semaphores strictly for monitoring purposes (sem_getvalue)
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

    // 2. Setup Shared Memory (Assigning to GLOBAL variables)
    // We use O_CREAT just in case the manager is started before anything else
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) { 
        perror("shm_open failed"); 
        exit(EXIT_FAILURE); 
    }

    if (ftruncate(shm_fd, sizeof(SharedMemory)) == -1) { 
        perror("ftruncate"); 
        exit(EXIT_FAILURE); 
    }

    shm_ptr = mmap(NULL, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) { 
        perror("mmap failed"); 
        exit(EXIT_FAILURE); 
    }

    // 3. Setup Semaphores (Assigning to GLOBAL variables)
    mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);
    empty = sem_open(SEM_EMPTY, O_CREAT, 0666, K);
    full  = sem_open(SEM_FULL,  O_CREAT, 0666, 0);
    
    if (mutex == SEM_FAILED || empty == SEM_FAILED || full == SEM_FAILED) {
        perror("sem_open failed");
        exit(EXIT_FAILURE);
    }

    printf("[Manager PID: %d] Started successfully. Press Ctrl+C to exit.\n", getpid());
    printf("[Manager] I will perform aging and monitoring every 5 seconds.\n");

    // 4. Main Manager Loop
    while (1) {
        // Sleep for 5 seconds as requested by the task
        sleep(5);

        // We ONLY lock the mutex. We do not touch 'empty' or 'full' because
        // we are not changing the total number of tasks in the buffer.
        sem_wait(mutex);

        // --- CRITICAL SECTION START ---
        
        printf("\n================ SYSTEM MONITOR ================\n");
        
        // Anti-Starvation (Aging) Logic
        if (shm_ptr->normal_count > 0) {
            // 1. Copy the oldest task from the NORMAL queue
            Task aging_task;
            strncpy(aging_task.text, shm_ptr->normal_queue[shm_ptr->normal_head].text, TASK_LEN);
            
            // 2. Remove it from the NORMAL queue
            shm_ptr->normal_head = (shm_ptr->normal_head + 1) % K;
            shm_ptr->normal_count--;

            // 3. Append it to the PRIORITY queue
            strncpy(shm_ptr->priority_queue[shm_ptr->priority_tail].text, aging_task.text, TASK_LEN);
            shm_ptr->priority_tail = (shm_ptr->priority_tail + 1) % K;
            shm_ptr->priority_count++;

            printf("[ACTION] Starvation prevented: 1 task moved from NORMAL -> PRIORITY.\n");
        } else {
            printf("[ACTION] No NORMAL tasks waiting. No aging required.\n");
        }

        // Monitoring Logic
        int empty_val, full_val;
        sem_getvalue(empty, &empty_val);
        sem_getvalue(full, &full_val);

        int total_tasks = shm_ptr->priority_count + shm_ptr->normal_count;

        printf("  Total System Capacity: %d\n", K);
        printf("  Free Slots (Empty):    %d\n", empty_val);
        printf("  Total Tasks (Full):    %d\n", full_val);
        printf("  --------------------------------------\n");
        printf("  Tasks in PRIORITY:     %d\n", shm_ptr->priority_count);
        printf("  Tasks in NORMAL:       %d\n", shm_ptr->normal_count);
        
        // Safety check to ensure data consistency
        if (total_tasks != full_val) {
            printf("  [WARNING] Data inconsistency detected!\n");
        }
        printf("================================================\n");

        // --- CRITICAL SECTION END ---

        sem_post(mutex);
    }

    return EXIT_SUCCESS;
}
