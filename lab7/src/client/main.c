#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> /* Required for modern inet_pton() */

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int socket_fd;
    struct sockaddr_in server_address;
    
    char tx_buffer[BUFFER_SIZE];
    char rx_buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    /* * STEP 1: Validate command-line arguments.
     * Expected format: ./client <IP_ADDRESS> <PORT> <NUMBER>
     * argc must be exactly 4 (program name + 3 arguments).
     */
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <Server_IPv4> <Port> <Number_To_Add>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* * STEP 2: Create the client socket.
     * We use SOCK_STREAM for TCP and SOCK_CLOEXEC as a modern safety measure.
     */
    socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (socket_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    /* * STEP 3: Configure the target server address structure.
     * - sin_family: Always AF_INET for IPv4.
     * - inet_pton: Modern, safe function to convert the text IP (e.g., "127.0.0.1")
     * into the binary network byte format. Returns 1 on success.
     * - htons + atoi: Convert the text port argument into a 16-bit integer 
     * and flip its endianness to Network Byte Order.
     */
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    
    if (inet_pton(AF_INET, argv[1], &server_address.sin_addr) <= 0) {
        fprintf(stderr, "Error: Invalid IPv4 address provided: %s\n", argv[1]);
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
    
    server_address.sin_port = htons(atoi(argv[2]));

    /* * STEP 4: Establish the TCP connection with the server.
     * This call triggers the 3-way handshake under the hood.
     * We cast 'struct sockaddr_in*' to the generic 'struct sockaddr*' type.
     */
    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection to the server failed");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    /* * STEP 5: Format and send the protocol message.
     * We prepare the exact "ZADANIE LICZBA" format required by the task description.
     */
    snprintf(tx_buffer, sizeof(tx_buffer), "ZADANIE %s", argv[3]);
    
    /* MSG_NOSIGNAL prevents the application from crashing if the server dropped early */
    if (send(socket_fd, tx_buffer, strlen(tx_buffer), MSG_NOSIGNAL) < 0) {
        perror("Failed to send data to the server");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    /* * STEP 6: Receive the text response from the server.
     * We leave 1 byte at the end of the buffer for the null-terminator.
     */
    memset(rx_buffer, 0, sizeof(rx_buffer));
    bytes_received = recv(socket_fd, rx_buffer, sizeof(rx_buffer) - 1, 0);
    
    if (bytes_received < 0) {
        perror("Failed to receive data from the server");
        close(socket_fd);
        exit(EXIT_FAILURE);
    } else if (bytes_received == 0) {
        fprintf(stderr, "Error: Server closed the connection unexpectedly.\n");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    /* * STEP 7: Safe text processing and output.
     * Null-terminate the raw network bytes to safely print them to stdout.
     */
    rx_buffer[bytes_received] = '\0';
    printf("%s\n", rx_buffer);

    /* * STEP 8: Clean up.
     * Close the descriptor and return success.
     */
    close(socket_fd);
    return 0;
}