#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "socket_layer.h"
#include <sys/types.h>      // Needed for using various data types and structs
#include <sys/socket.h>     // Needed for socket functions
#include "error.h"
#include <netinet/in.h>     // Needed for sockaddr_in and INADDR_ANY


// Function to initialize a TCP server on a specified port
int tcp_server_init(uint16_t port) {
    // Create a socket
    int socketID = socket(AF_INET, SOCK_STREAM, 0);
    if (socketID == -1) {
        perror("fail in socket creation");
        return ERR_IO;// Return error if socket creation fails
    }

    // Set up the server address structure
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;    // Set address family to AF_INET
    server_address.sin_port = htons(port);  // Set port number, converting to network byte order
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // Listen on all interfaces


    // Bind the socket to the specified port and address
    if (bind(socketID, (struct sockaddr *) &server_address, sizeof(server_address)) == -1) {
        perror("fail in binding");
        return ERR_IO;  // Return error if binding fails
    }

    // Listen for incoming connections
    if (listen(socketID, 1) == -1) {
        perror("fail in listening");
        return ERR_IO;  // Return error if listen fails
    }

    return socketID;    // Return the socket ID if successful
}

// Function to accept a new connection on a passive socket
int tcp_accept(int passive_socket) {
    int active_socket = accept(passive_socket, NULL, NULL);
    if (active_socket < 0) {
        perror("fail in accepting");
        return ERR_IO;  // Return error if accept fails
    }
    return active_socket;  // Return the new socket ID for the accepted connection
}

ssize_t tcp_read(int active_socket, char *buf, size_t buflen) {

    if (active_socket < 0) {
        return ERR_INVALID_ARGUMENT;    // Return error if socket ID is invalid
    }
    if (buf == NULL) {
        return ERR_INVALID_ARGUMENT;    // Return error if buffer is NULL
    }
    if (buflen <= 0) {
        return ERR_INVALID_ARGUMENT;    // Return error if buffer length is invalid
    }

    // Read data from the socket
    ssize_t bytes_read = recv(active_socket, buf, buflen, 0);
    if (bytes_read == -1) {
        perror("fail in reading");
        return ERR_IO;  // Return error if read fails
    }
    return bytes_read;  // Return the number of bytes read
}



// Function to send data to an active socket
ssize_t tcp_send(int active_socket, const char *response, size_t response_len) {
    if (active_socket < 0) {
        return ERR_INVALID_ARGUMENT;    // Return error if socket ID is invalid
    }
    if (response == NULL) {
        return ERR_INVALID_ARGUMENT;    // Return error if response buffer is NULL
    }
    if (response_len <= 0) {
        return ERR_INVALID_ARGUMENT;    // Return error if response length is invalid
    }

    // Send data to the socket
    ssize_t bytes_sent = send(active_socket, response, response_len, 0);
    if (bytes_sent == -1) {
        perror("fail in sending");
        return ERR_IO;  // Return error if send fails
    }
    return bytes_sent;  // Return the number of bytes sent
}