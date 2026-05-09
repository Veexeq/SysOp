#include "common.h"
#include "client.h"

// Global variables for the signal handler
mqd_t client_q = (mqd_t) -1;
char client_q_name[MAX_Q_NAME];
pid_t child_pid = -1;

void cleanup_handler(int sig) {
    printf("\n[CLIENT] has recieved a signal %d. Closing the client (ID = %d) and doing a cleanup of the IPC queues...\n", sig, (int) client_q);

    if (client_q != (mqd_t) -1) {
        mq_close(client_q);
    }
    mq_unlink(client_q_name);

    if (child_pid > 0) {
        kill(child_pid, SIGTERM);
    }

    printf("[CLIENT] Cleanup was a success. Exit.\n");
    exit(EXIT_SUCCESS);
}

int main(void) {
    signal(SIGINT, cleanup_handler);
    signal(SIGTERM, cleanup_handler);

    // Generating an unique name using PID
    snprintf(client_q_name, MAX_Q_NAME, "/chat_client_%d", getpid());

    struct mq_attr attr = {0};
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MSG;
    attr.mq_msgsize = sizeof(ChatMessage);
    attr.mq_curmsgs = 0;

    // Creating a read-only queue for the server to write to
    client_q = mq_open(client_q_name, O_CREAT | O_RDONLY, 0666, &attr);
    if (client_q == (mqd_t) -1) {
        fprintf(stderr, "Couldn't open the client's private queue (PID: %d).\n", (int) getpid());
        exit(EXIT_FAILURE);
    }

    // Creating a write-only queue to the server
    mqd_t server_q = mq_open(SERVER_QUEUE_NAME, O_WRONLY);
    if (server_q == (mqd_t) -1) {
        fprintf(stderr, "Client (PID: %d) couldn't open a queue to the server. Check whether it's opened.\n", (int) getpid());
        cleanup_handler(SIGTERM);
    }

    // Initialize the connection with the server via an INIT message
    ChatMessage init_msg = {0};
    init_msg.type = MSG_INIT;
    strncpy(init_msg.queue_name, client_q_name, MAX_Q_NAME);

    if (mq_send(server_q, (const char *) &init_msg, sizeof(ChatMessage), 0) == -1) {
        perror("Error when sending MSG_INIT\n");
        cleanup_handler(SIGTERM);
    }

    printf("Sent an INIT message to the server.\n");

    // Wait for the reply
    ChatMessage reply_msg = {0};
    if (mq_receive(client_q, (char *) &reply_msg, sizeof(ChatMessage), NULL) == -1) {
        perror("Encountered an error upon recieveing a message from the server. Shutdown.\n");
        cleanup_handler(SIGTERM);
    }

    int my_id = -1;
    if (reply_msg.type == MSG_ID_ASSIGN) {
        my_id = reply_msg.client_id;
        printf("Connected to the server. Given ID: %d\n", my_id);
    } else {
        fprintf(stderr, "Server responded with something else than MSG_ID_ASSIGN. Shutdown.\n");
        cleanup_handler(SIGTERM);
    }

    printf("You now can write messages to others (press CTRL+C to exit)\n");
    printf("-----------------------------------------------------------\n");

    child_pid = fork();
    if (child_pid == -1) {
        perror("An error encountered when called fork()\n");
        cleanup_handler(SIGTERM);
    }

    if (child_pid == 0) {
        // Child's path: the receiver
        ChatMessage incoming_msg;
        while (1) {
            ssize_t bytes = mq_receive(client_q, (char *) &incoming_msg, sizeof(ChatMessage), NULL);
            if (bytes > 0 && incoming_msg.type == MSG_TEXT) {
                printf("\r[CLIENT %d]: %s", incoming_msg.client_id, incoming_msg.text);
                printf("\r> ");
                fflush(stdout); 
            }
        }
    } else {
        // Parent's path: the sender
        ChatMessage out_msg = {0};
        out_msg.type = MSG_TEXT;
        out_msg.client_id = my_id;

        while (1) {
            printf("\r> ");
            fflush(stdout);

            if (fgets(out_msg.text, MAX_TEXT, stdin) != NULL) {
                // Do not send empty messages
                if (strcmp(out_msg.text, "\n") == 0) {
                    continue;
                }

                if (mq_send(server_q, (const char *) &out_msg, sizeof(ChatMessage), 0) == -1) {
                    perror("Error encountered upon trying to send a message\n");
                }
            }
        }
    }

    return EXIT_SUCCESS;
}
