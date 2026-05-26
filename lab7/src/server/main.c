#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>

#define SERVER_PORT 9000

/* * Defines the maximum number of pending connections in the queue.
 * These are connections that have completed the TCP handshake but 
 * have not been accepted by the application yet.
 */
#define BACKLOG_SIZE 10

#define BUFFER_SIZE 1024

/* * Global flag used to control the main server loop execution.
 * volatile: Tells the compiler not to optimize this variable into a register.
 * sig_atomic_t: Guaranteed to be accessed atomically even during an interrupt.
 */
volatile sig_atomic_t keep_running = 1;

/* * Signal handler function for SIGINT (Ctrl+C).
 * This function must execute as fast as possible and only perform safe operations.
 */
void handle_sigint(int sig) {
    (void)sig; /* Suppress unused parameter warning */
    keep_running = 0; /* Just flip the switch, let main handle the cleanup */
}

int main(void) {
    int server_fd;
    struct sockaddr_in server_address;
    int opt_reuse = 1;

    /* Global request counter required by the assignment task */
    int request_counter = 0;

    struct sigaction sa;

    /* ==============================================================================
     * SIGNAL CONFIGURATION (Modern sigaction approach)
     * ============================================================================== */
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint; /* Point to our custom handler function */
    sigemptyset(&sa.sa_mask);      /* Do not block any other signals during execution */
    sa.sa_flags = 0;               /* Do NOT use SA_RESTART. We WANT system calls to fail with EINTR */

    if (sigaction(SIGINT, &sa, NULL) < 0) {
        perror("Failed to register SIGINT handler");
        exit(EXIT_FAILURE);
    }

    /* * STEP 1: Create the socket descriptor.
     * - AF_INET: Specifies the IPv4 protocol family.
     * - SOCK_STREAM: Specifies sequential, reliable TCP byte stream.
     * - SOCK_CLOEXEC: Modern best practice. Automatically closes this file 
     * descriptor if the process executes another program via execve().
     */
    server_fd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    /* * STEP 2: Configure socket options.
     * - SOL_SOCKET: Specifies that the option is at the socket API level.
     * - SO_REUSEADDR: Allows the socket to bind to an address/port that is 
     * in the TIME_WAIT state. Prevents the "Address already in use" error 
     * when restarting the server during debugging.
     */
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_reuse, sizeof(opt_reuse)) < 0) {
        perror("Setting SO_REUSEADDR failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    /* * STEP 3: Prepare the server address structure.
     * - memset: Zeroes out the memory to prevent garbage data.
     * - sin_family: Must match the domain specified in socket().
     * - sin_addr.s_addr: htonl(INADDR_ANY) configures the server to listen 
     * on ALL available network interfaces (WiFi, Ethernet, Localhost).
     * - sin_port: htons() converts the port number from Host Byte Order (Little-Endian)
     * to Network Byte Order (Big-Endian).
     */
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(SERVER_PORT);

    /* * STEP 4: Bind the socket to the specified network interface and port.
     * We cast the specific 'struct sockaddr_in' pointer to the generic 
     * 'struct sockaddr' pointer to satisfy the system function requirements.
     */
    if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Bind operation failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    /* * STEP 5: Put the socket into listening mode.
     * This tells the operating system kernel that the server is now active 
     * and ready to queue up incoming client connection requests.
     */
    if (listen(server_fd, BACKLOG_SIZE) < 0) {
        perror("Listen operation failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("SUCCESS: Server is fully operational and listening on port %d...\n", SERVER_PORT);

    /* ==============================================================================
     * INFINITE SERVER LOOP (Iterative approach)
     * ============================================================================== */
    while (keep_running) {
        int client_fd;
        struct sockaddr_in client_address;
        socklen_t client_addr_len = sizeof(client_address);
        
        char rx_buffer[BUFFER_SIZE];
        char tx_buffer[BUFFER_SIZE * 2];
        char http_body[BUFFER_SIZE];
        
        ssize_t bytes_received;

        /* * STEP 1: Wait and accept an incoming connection.
         * accept4() blocks the execution until a client connects.
         * SOCK_CLOEXEC ensures the client socket descriptor is not leaked to child processes.
         */
        client_fd = accept4(server_fd, (struct sockaddr *)&client_address, &client_addr_len, SOCK_CLOEXEC);
        if (client_fd < 0) {
            perror("Accept failed");
            continue; /* Do not crash the server, just wait for the next client */
        }

        /* * STEP 2: Read data sent by the client.
         * We reserve the last byte of our buffer for the null-terminator '\0'.
         */
        memset(rx_buffer, 0, sizeof(rx_buffer));
        bytes_received = recv(client_fd, rx_buffer, sizeof(rx_buffer) - 1, 0);
        
        if (bytes_received < 0) {
            perror("Recv failed");
            close(client_fd);
            continue;
        } else if (bytes_received == 0) {
            /* Client disconnected immediately without sending data */
            close(client_fd);
            continue;
        }

        /* * CRITICAL STEP FOR C: Null-terminate the received data.
         * Network functions send raw bytes, not C-style strings. Without '\0', 
         * string functions like strncmp() or sscanf() would read out of bounds.
         */
        rx_buffer[bytes_received] = '\0';

        /* Print the first line of the received request to the server terminal */
        printf("--- NEW REQUEST RECEIVED ---\n%s\n----------------------------\n", rx_buffer);

        /* * STEP 3: Parse the request and execute business logic.
         * We look for HTTP methods (GET/POST) or our custom command (ZADANIE).
         */
        if (strncmp(rx_buffer, "GET", 3) == 0 || strncmp(rx_buffer, "POST", 4) == 0) {
            
            /* Increment counter for valid HTTP requests */
            request_counter++;

            /* 1. Generate the response text body first, so we can calculate its exact length */
            int body_len = snprintf(http_body, sizeof(http_body), "Liczba pobrań strony: %d", request_counter);

            /* 2. Assemble the complete HTTP response including headers and the body */
            snprintf(tx_buffer, sizeof(tx_buffer),
                     "HTTP/1.1 200 OK\r\n"
                     "Server: Zajeciowy serwer SO\r\n"
                     "Content-Type: text/plain; charset=utf-8\r\n"
                     "Connection: close\r\n"
                     "Cache-Control: no-store\r\n"
                     "Content-Length: %d\r\n\r\n"
                     "%s", 
                     body_len, http_body);

            /* 3. Send the formatted HTTP package back to the client */
            send(client_fd, tx_buffer, strlen(tx_buffer), MSG_NOSIGNAL);

        } else if (strncmp(rx_buffer, "ZADANIE", 7) == 0) {
            int task_value = 0;

            /* Extract the integer value following the "ZADANIE" keyword */
            if (sscanf(rx_buffer, "ZADANIE %d", &task_value) == 1) {
                request_counter += task_value;
            }

            /* Format response text WITHOUT any HTTP headers, as requested */
            snprintf(tx_buffer, sizeof(tx_buffer), "Liczba pobrań strony: %d", request_counter);

            /* Send raw text response */
            send(client_fd, tx_buffer, strlen(tx_buffer), MSG_NOSIGNAL);
        }

        /* * STEP 4: Close the connection.
         * This triggers the TCP FIN handshake, releasing the client socket resources.
         */
        close(client_fd);
    }

    /* ==============================================================================
     * SAFE CLEANUP SECTION
     * ============================================================================== */
    printf("Closing server listening socket and releasing resources...\n");
    close(server_fd);
    
    printf("Final application state: REQUEST_COUNTER = %d\n", request_counter);
    printf("Server shut down gracefully. Goodbye!\n");
}