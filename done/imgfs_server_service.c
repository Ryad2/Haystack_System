/*
 * @file imgfs_server_services.c
 * @brief ImgFS server part, bridge between HTTP server layer and ImgFS library
 *
 * @author Konstantinos Prasopoulos
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> // uint16_t

#include "error.h"
#include "util.h" // atouint16
#include "imgfs.h"
#include "http_net.h"
#include "imgfs_server_service.h"
#include <vips/vips.h>

// Main in-memory structure for imgFS
static struct imgfs_file fs_file;
static uint16_t server_port;

#define URI_ROOT "/imgfs"

/********************************************************************//**
 * Startup function. Create imgFS file and load in-memory structure.
 * Pass the imgFS file name as argv[1] and optionnaly port number as argv[2]
 ********************************************************************** */
int server_startup (int argc, char **argv)
{
    if (argc < 2) return ERR_NOT_ENOUGH_ARGUMENTS;
    int errcode = ERR_NONE;
    if ((errcode = do_open(argv[1], "rb", &fs_file))) {
        return errcode;
    }
    print_header(&fs_file.header);

    server_port = (uint16_t) http_init(argc > 2 ? atouint16(argv[2]) : DEFAULT_LISTENING_PORT, handle_http_message);

    if (VIPS_INIT(argv[0])) {
        return vips_error_exit(NULL);
    }

    printf("ImgFS server started on http://localhost:%d\n", server_port);
    fflush(stdout);

    return ERR_NONE;
}

/********************************************************************//**
 * Shutdown function. Free the structures and close the file.
 ********************************************************************** */
void server_shutdown (void)
{
    fprintf(stderr, "Shutting down...\n");
    http_close();
    do_close(&fs_file);
    vips_shutdown();
    return 0;
}










/**********************************************************************
 * Sends error message.
 ********************************************************************** */
static int reply_error_msg(int connection, int error)
{
#define ERR_MSG_SIZE 256
    char err_msg[ERR_MSG_SIZE]; // enough for any reasonable err_msg
    if (snprintf(err_msg, ERR_MSG_SIZE, "Error: %s\n", ERR_MSG(error)) < 0) {
        fprintf(stderr, "reply_error_msg(): sprintf() failed...\n");
        return ERR_RUNTIME;
    }
    return http_reply(connection, "500 Internal Server Error", "",
                      err_msg, strlen(err_msg));
}

/**********************************************************************
 * Sends 302 OK message.
 ********************************************************************** */
static int reply_302_msg(int connection)
{
    char location[ERR_MSG_SIZE];
    if (snprintf(location, ERR_MSG_SIZE, "Location: http://localhost:%d/" BASE_FILE HTTP_LINE_DELIM,
                 server_port) < 0) {
        fprintf(stderr, "reply_302_msg(): sprintf() failed...\n");
        return ERR_RUNTIME;
    }
    return http_reply(connection, "302 Found", location, "", 0);
}

/**********************************************************************
 * Simple handling of http message. TO BE UPDATED WEEK 13
 ********************************************************************** */
int handle_http_message(struct http_message* msg, int connection)
{
    M_REQUIRE_NON_NULL(msg);

    if (http_match_verb(&msg->uri, "/") || http_match_uri(msg, "/index.html")) {
        return http_serve_file(connection, BASE_FILE);
    }

    debug_printf("handle_http_message() on connection %d. URI: %.*s\n",
                 connection,
                 (int) msg->uri.len, msg->uri.val);

    if (http_match_uri(msg, URI_ROOT "/list")) {
        return handle_list_call(msg, connection);
    }
    else if (http_match_uri(msg, URI_ROOT "/insert") && http_match_verb(&msg->method, "POST")) {
        return handle_insert_call(msg, connection);
    }
    else if (http_match_uri(msg, URI_ROOT "/read")) {
        return handle_read_call(msg, connection);
    }
    else if (http_match_uri(msg, URI_ROOT "/delete")) {
        return handle_delete_call(msg, connection);
    } 
    else
        return reply_error_msg(connection, ERR_INVALID_COMMAND);
}

int handle_list_call(struct http_message* msg, int connection) {
    char* output = NULL;
    int errcode = 0;
    if ((errcode = do_list(&fs_file, JSON, &output))) {
        return reply_error_msg(connection, errcode);
    }

    return http_reply(connection, HTTP_OK, "Content-Type: application/json" HTTP_LINE_DELIM, output, sizeof(output));
}

int handle_read_call(struct http_message* msg, int connection) {
    int errcode = 0;
    char res[6] = {0};
    if ((errcode = http_get_var(&msg->uri, "res", res, 6))) {
        return reply_error_msg(connection, errcode);
    }
    char img_id[MAX_IMG_ID] = {0};
    if (( errcode = http_get_var(&msg->uri, "img_id", img_id, MAX_IMG_ID))) {
        return reply_error_msg(connection, errcode);
    }

    char* buffer = NULL;
    uint32_t size = 0;
    if ((errcode = do_read(img_id, atoi(res), &buffer, &size, &fs_file))) {
        return reply_error_msg(connection, errcode);
    }
    return http_reply(connection, HTTP_OK, "Content-Type: image/jpeg" HTTP_LINE_DELIM, buffer, size);
}

int handle_delete_call(struct http_message* msg, int connection) {
    int errcode = 0;
    char img_id[MAX_IMG_ID] = {0};
    if (( errcode = http_get_var(&msg->uri, "img_id", img_id, MAX_IMG_ID))) {
        return reply_error_msg(connection, errcode);
    }

    if ((errcode = do_delete(img_id, &fs_file))) {
        return reply_error_msg(connection, errcode);
    }

    char port_string[50] = {0};
    sprintf(port_string, "Location: http://" "localhost:%d/index.html", server_port);
    return http_reply(connection, "302 Found", port_string, NULL, 0);

}

int handle_insert_call(struct http_message* msg, int connection) {
    int errcode = 0;
    char name[MAX_IMG_ID+1] = {0};
    if ((errcode = http_get_var(&msg->uri, "name", name, 6))) {
        return reply_error_msg(connection, errcode);
    }

    size_t size = msg->body.len;
    char* buffer = calloc(1, size);
    if (buffer == NULL) {
        return reply_error_msg(connection, ERR_OUT_OF_MEMORY);
    }
    memcpy(buffer, msg->body.val, size);

    if ((errcode = do_insert(buffer, size, name, &fs_file))) {
        return reply_error_msg(connection, errcode);
    }

    char port_string[50] = {0};
    sprintf(port_string, "Location: http://" "localhost:%d/index.html", server_port);
    return http_reply(connection, HTTP_OK, port_string, NULL, 0);
}