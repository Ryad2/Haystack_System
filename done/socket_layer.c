#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "socket_layer.h"
#include <sys/types.h>      // Needed for using various data types and structs
#include <sys/socket.h>     // Needed for socket functions
#include "error.h"
#include <netinet/in.h>




int tcp_server_init(uint16_t port) {
    int socketID = socket(AF_INET, SOCK_STREAM, 0);
    if (socketID == -1) {
        perror("fail in socket creation");
        return ERR_IO;
    }
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);  //todo just browsing Listen on all interfaces

    if (bind(socketID, (struct sockaddr *) &server_address, sizeof(server_address)) == -1) {
        perror("fail in binding");
        return ERR_IO;
    }

    if (listen(socketID, 1) == -1) {
        perror("fail in listening");
        return ERR_IO;
    }

    return socketID;
}


int tcp_accept(int passive_socket) {
    int active_socket = accept(passive_socket, NULL, NULL);
    if (active_socket == -1) {
        perror("fail in accepting");
        return ERR_IO;
    }
    return active_socket;
}

ssize_t tcp_read(int active_socket, char* buf, size_t buflen) {
    if (active_socket == -1) {
        return ERR_INVALID_ARGUMENT;
    }
    if(buf == NULL) {
        return ERR_INVALID_ARGUMENT;
    }
    if(buflen == 0) {
        return ERR_INVALID_ARGUMENT;
    }

    ssize_t bytes_read = recv(active_socket, buf, buflen, 0);
    if (bytes_read == -1) {
        perror("fail in reading");
        return ERR_IO;
    }
    return bytes_read;
}

ssize_t tcp_send(int active_socket, const char* response, size_t response_len) {
    if (active_socket == -1) {
        return ERR_INVALID_ARGUMENT;
    }
    if(response == NULL) {
        return ERR_INVALID_ARGUMENT;
    }
    if(response_len == 0) {
        return ERR_INVALID_ARGUMENT;
    }

    ssize_t bytes_sent = send(active_socket, response, response_len, 0);
    if (bytes_sent == -1) {
        perror("fail in sending");
        return ERR_IO;
    }
    return bytes_sent;
}