#include "common.h"
#include "server.h"

mqd_t server_q = (mqd_t) -1;

void cleanup_handler(int sig) {
    printf("\n[SERVER] has recieved a signal %d. Closing the server and doing a cleanup of the IPC queues...\n", sig);

    if (server_q != (mqd_t) -1) {
        mq_close(server_q);
    }
    mq_unlink(SERVER_QUEUE_NAME);

    printf("[SERVER] Cleanup was a success. Exit.\n");
    exit(EXIT_SUCCESS);
}

int main(void) {
    signal(SIGINT, cleanup_handler);
    signal(SIGTERM, cleanup_handler);

    mqd_t client_queues[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        client_queues[i] = (mqd_t) -1;
    }

    int active_clients = 0;

    // Config of the server queue's attributes
    struct mq_attr attr = {0};
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MSG;
    attr.mq_msgsize = sizeof(ChatMessage);
    attr.mq_curmsgs = 0;

    // Remove the previous queue, if having encountered a bug, 
    // the previous instance of the server didn't close it
    mq_unlink(SERVER_QUEUE_NAME);

    server_q = mq_open(SERVER_QUEUE_NAME, O_CREAT | O_RDONLY, 0666, &attr);
    if (server_q == (mqd_t) -1) {
        perror("Couldn't create the server's queue\n");
        exit(EXIT_FAILURE);
    }

    printf("Server up. Waiting for messages...\n");

    ChatMessage msg;
    while (1) {
        ssize_t bytes_read = mq_receive(server_q, (char *) &msg, sizeof(ChatMessage), NULL);
        
        if (bytes_read < 0) {
            perror("An error occured when reading a message with mq_recieve.\n");
            continue;
        }

        if (msg.type == MSG_INIT) {
            // Initialize the client
            if (active_clients < MAX_CLIENTS) {
                int new_id = active_clients;
                mqd_t client_q = mq_open(msg.queue_name, O_WRONLY);

                if (client_q != (mqd_t) -1) {
                    client_queues[new_id] = client_q;
                    active_clients++;
                
                    // Prepare the answear with `new_id` for the client
                    ChatMessage reply = {0};
                    reply.type = MSG_ID_ASSIGN;
                    reply.client_id = new_id;

                    mq_send(client_q, (const char *) &reply, sizeof(ChatMessage), 0);
                    printf("Registered client with an ID: %d (queue: %s)\n", new_id, msg.queue_name);
                } else {
                    fprintf(stderr, "Couldn't open a client's queue (queue_name: %s)\n", msg.queue_name);
                    continue;
                }
            } else {
                fprintf(stderr, "A maximum of clients (%d) has been reached. Couldn't one more.\n", MAX_CLIENTS);
            }
        } else if (msg.type == MSG_TEXT) {
            printf("Recieved a message from ID: %d. Text: %s\n", msg.client_id, msg.text);
            
            // Send the message forward to others
            for (int i = 0; i < active_clients; ++i) {
                if (i != msg.client_id && client_queues[i] != (mqd_t) -1) {
                    mq_send(client_queues[i], (const char *) &msg, sizeof(ChatMessage), 0);
                }
            }
        }
    }

    exit(EXIT_SUCCESS);
}
