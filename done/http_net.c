/*
 * @file http_net.c
 * @brief HTTP server layer for CS-202 project
 *
 * @author Konstantinos Prasopoulos
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>

#include "http_prot.h"
#include "http_net.h"
#include "socket_layer.h"
#include "error.h"

static int passive_socket = -1;
static EventCallback cb;

#define MK_OUR_ERR(X) \
static int our_ ## X = X

MK_OUR_ERR(ERR_NONE);
MK_OUR_ERR(ERR_INVALID_ARGUMENT);
MK_OUR_ERR(ERR_OUT_OF_MEMORY);
MK_OUR_ERR(ERR_IO);

/*******************************************************************
 * Handle connection
 */
static void *handle_connection(void *arg)
{
    if (arg == NULL) {
        return &our_ERR_INVALID_ARGUMENT;
    }


    int *active_socket = (int *) arg;
    char *buffer = calloc(1, MAX_HEADER_SIZE + 1);  // Allocate buffer
    if (buffer == NULL) {
        return &our_ERR_OUT_OF_MEMORY;
    }

    int bytes_received = 0;
    int content_len = 0;
    struct http_message msg;
    memset(&msg, 0, sizeof(msg));

    while (1) {
        int n = tcp_read(*active_socket, buffer + bytes_received, MAX_HEADER_SIZE - bytes_received);
        if (n <= 0) {
            free(buffer);
            return &our_ERR_IO;
        }

        bytes_received += n;
        buffer[bytes_received] = '\0';

        // Try to parse the message
        int parse_result = http_parse_message(buffer, bytes_received, &msg, &content_len);
        if (parse_result < 0) {
            free(buffer);
            return &parse_result;//todo returning a pointer to an int that been declared in the same function
        }

        if (parse_result == 1) {
            // Full message has been parsed
            break;
        }

        // If message is incomplete, continue reading
        if (bytes_received >= MAX_HEADER_SIZE) {
            free(buffer);
            return &our_ERR_INVALID_ARGUMENT;
        }
    }

    // Call the HTTP message handler
    int handler_result = handle_http_message(&msg, *active_socket);
    if (handler_result != ERR_NONE) {
        free(buffer);
        return &handler_result;//todo returning a pointer to an int that been declared in the same function
    }

    free(buffer);
    return &our_ERR_NONE;







    //  PREVIOUS CODE
    /*
    int* active_socket = (int*) arg;
    const char* testOk = "test: ok";
    char* endPtr = NULL;
    int hasSeenTestOk = 0;

    char* buffer = calloc(1, MAX_HEADER_SIZE + 1);
    if (buffer == NULL) {
        return &our_ERR_OUT_OF_MEMORY;
    }

    while(endPtr == NULL && strlen(buffer) < MAX_HEADER_SIZE) {
        tcp_read(*active_socket, buffer, MAX_HEADER_SIZE - strlen(buffer));
        endPtr = strstr(buffer, HTTP_HDR_END_DELIM);

        if (strstr(buffer, testOk) != NULL) {
            hasSeenTestOk = 1;
        }
    }



    // TODO "handle error cases" I don't know what else can happen yet
    if (http_reply(*active_socket, hasSeenTestOk ? HTTP_OK : HTTP_BAD_REQUEST, NULL, NULL, 0)) {
        return &our_ERR_IO;
    }
    
    return &our_ERR_NONE;
    */
}


/*******************************************************************
 * Init connection
 */
int http_init(uint16_t port, EventCallback callback)
{
    passive_socket = tcp_server_init(port);
    cb = callback;
    return passive_socket;
}

/*******************************************************************
 * Close connection
 */
void http_close(void)
{
    if (passive_socket > 0) {
        if (close(passive_socket) == -1)
            perror("close() in http_close()");
        else
            passive_socket = -1;
    }
}

/*******************************************************************
 * Receive content
 */
int http_receive(void)
{
    int active_socket = tcp_accept(passive_socket);
    if (active_socket < 0) {
        return active_socket;
    }

    handle_connection(&active_socket);
    return ERR_NONE;
}

/*******************************************************************
 * Serve a file content over HTTP
 */
int http_serve_file(int connection, const char* filename)
{
    int ret = ERR_NONE;
    return ret;
}

/*******************************************************************
 * Create and send HTTP reply
 */
int http_reply(int connection, const char* status, const char* headers, const char *body, size_t body_len)
{
    if (body_len > 0 && body == NULL) {
        return ERR_INVALID_ARGUMENT;
    }

    const char contentLengthStr[] = "Content-Length: ";
    char body_lenStr[10] = {0}; 
    sprintf(body_lenStr, "%zu", body_len);
    const char* headerElements[] = {HTTP_PROTOCOL_ID, status, HTTP_LINE_DELIM, headers, contentLengthStr, body_lenStr, HTTP_HDR_END_DELIM};

    size_t header_len = 0;
    for (int i = 0; i < 7; ++i) {
        if (headerElements[i] != NULL) {
            header_len += strlen(headerElements[i]);
        }
    }

    size_t len = header_len + body_len + 1;
    char* resp = calloc(1, len);
    if (resp == NULL) {
        return ERR_OUT_OF_MEMORY;
    }

    for (int i = 0; i < 7; ++i) {
        if (headerElements[i] != NULL) {
            strcat(resp, headerElements[i]);
        }
    }

    if (body != NULL) {
        strcat(resp, body);
    }
    
    tcp_send(connection, resp, len);
    free(resp);

    return ERR_NONE;
}
