#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* Define the port on which the server will listen */
#define SERVER_PORT 9000

/* * Defines the maximum number of pending connections in the queue.
 * These are connections that have completed the TCP handshake but 
 * have not been accepted by the application yet.
 */
#define BACKLOG_SIZE 10

int main(void) {
    int server_fd;
    struct sockaddr_in server_address;
    int opt_reuse = 1;

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

    printf("SUCCESS: Server is listening on port %d!\n", SERVER_PORT);
    printf("Temporary: Sleeping for 20 seconds. Open another terminal and run 'ss -tulpn'\n");
    
    /* Temporary sleep to keep the socket alive for your verification */
    sleep(20);

    /* Clean up the socket before exiting */
    close(server_fd);
    printf("Server shut down successfully.\n");

    return 0;
}