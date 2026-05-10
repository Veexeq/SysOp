#include "common.h"

int main() {
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_MUTEX);
    sem_unlink(SEM_EMPTY);
    sem_unlink(SEM_FULL);
    printf("All POSIX IPC objects unlinked.\n");
    return EXIT_SUCCESS;
}
