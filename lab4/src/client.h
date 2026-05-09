#ifndef CLIENT_H
#define CLIENT_H

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_MSG 10

void cleanup_handler(int);

#endif
