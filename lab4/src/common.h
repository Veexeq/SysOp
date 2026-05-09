#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>      // Flagi O_* (np. O_CREAT, O_WRONLY)
#include <sys/stat.h>   // Definicje praw dostępu (np. 0666)
#include <mqueue.h>     // Funkcje POSIX Message Queues
#include <unistd.h>     // Dla getpid(), fork()
#include <errno.h>      // Obsługa błędów

#define SERVER_QUEUE_NAME "/chat_server_q"
#define MAX_CLIENTS 10
#define MAX_TEXT 256
#define MAX_Q_NAME 64

typedef enum {
    MSG_INIT,
    MSG_ID_ASSIGN,
    MSG_TEXT,
} MessageType;

typedef struct ChatMessage {
    MessageType type;
    int client_id;
    char queue_name[MAX_Q_NAME];
    char text[MAX_TEXT];
} ChatMessage;

#endif