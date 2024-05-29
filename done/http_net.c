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
#include <pthread.h>

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
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT );
    sigaddset(&mask, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    if (arg == NULL) {
        return &our_ERR_INVALID_ARGUMENT;
    }


    int *active_socket = (int *) arg;
    char *buffer = calloc(1, MAX_HEADER_SIZE + 1);  // Allocate buffer
    if (buffer == NULL) {
        close(active_socket);
        free(active_socket);
        return &our_ERR_OUT_OF_MEMORY;
    }

    int bytes_received = 0;
    int content_len = 0;
    struct http_message msg;
    memset(&msg, 0, sizeof(msg));

    while (1) {
        int n = tcp_read(*active_socket, &buffer[bytes_received], MAX_HEADER_SIZE - bytes_received);
        if (n <= 0) {
            close(active_socket);
            free(buffer);
            free(active_socket);
            return &our_ERR_IO;
        }

        bytes_received += n;
        buffer[bytes_received] = '\0';

        // Try to parse the message
        int parse_result = http_parse_message(buffer, bytes_received, &msg, &content_len);
        if (parse_result < 0) {
            close(active_socket);
            free(buffer);
            free(active_socket);
            return &parse_result;//todo returning a pointer to an int that been declared in the same function
        }
        
        if (parse_result == 1) {
            // Call the HTTP message handler
            // int handler_result = 
            cb(&msg, *active_socket);
            /**if (handler_result != ERR_NONE) {
                close(active_socket);
                free(buffer);
                free(active_socket);
                return &handler_result;//todo returning a pointer to an int that been declared in the same function
            }**/

            // empties values for new message
            bytes_received = 0;
            content_len = 0;
            memset(buffer, 0, sizeof(*buffer));
            memset(&msg, 0, sizeof(msg));
        }

        // If message is incomplete, continue reading
        if (bytes_received >= MAX_HEADER_SIZE) {
            close(active_socket);
            free(buffer);
            free(active_socket);
            return &our_ERR_INVALID_ARGUMENT;
        }
    }

    close(active_socket);
    free(buffer);
    free(active_socket);
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
    int* active_socket = malloc(sizeof(int));
    if (active_socket == NULL) {
        return ERR_OUT_OF_MEMORY;
    }

    *active_socket = tcp_accept(passive_socket);
    if (*active_socket < 0) {
        free(active_socket);
        return ERR_IO;
    }

    pthread_attr_t attr;

    if (pthread_attr_init(&attr) || pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) {
        free(active_socket);
        return ERR_IO;
    }
    
    pthread_t thread;
    pthread_create(&thread, &attr, handle_connection, active_socket);

    pthread_attr_destroy(&attr);

    return ERR_NONE;
}

/*******************************************************************
 * Serve a file content over HTTP
 */
int http_serve_file(int connection, const char* filename)
{
    M_REQUIRE_NON_NULL(filename);

    // open file
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "http_serve_file(): Failed to open file \"%s\"\n", filename);
        return http_reply(connection, "404 Not Found", "", "", 0);
    }

    // get its size
    fseek(file, 0, SEEK_END);
    const long pos = ftell(file);
    if (pos < 0) {
        fprintf(stderr, "http_serve_file(): Failed to tell file size of \"%s\"\n",
                filename);
        fclose(file);
        return ERR_IO;
    }
    rewind(file);
    const size_t file_size = (size_t) pos;

    // read file content
    char* const buffer = calloc(file_size + 1, 1);
    if (buffer == NULL) {
        fprintf(stderr, "http_serve_file(): Failed to allocate memory to serve \"%s\"\n", filename);
        fclose(file);
        return ERR_IO;
    }

    const size_t bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read != file_size) {
        fprintf(stderr, "http_serve_file(): Failed to read \"%s\"\n", filename);
        fclose(file);
        return ERR_IO;
    }

    // send the file
    const int  ret = http_reply(connection, HTTP_OK,
                                "Content-Type: text/html; charset=utf-8" HTTP_LINE_DELIM,
                                buffer, file_size);

    // garbage collecting
    fclose(file);
    free(buffer);
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

    size_t len = header_len +1 + body_len +1;
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
        memcpy(&resp[header_len], body, body_len);
    }
    
    tcp_send(connection, resp, len);
    free(resp);

    return ERR_NONE;
}
