#include "http_prot.h"
#include "error.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


int http_match_uri(const struct http_message *message, const char *target_uri){
    M_REQUIRE_NON_NULL(message);
    M_REQUIRE_NON_NULL(target_uri);

    if (message->uri.len < strlen(target_uri)) {
        return 0;
    }

    for (size_t i = 0; i < strlen(target_uri); ++i) {
        if (message->uri.val[i] != target_uri[i]) {
            return 0;
        }
    }

    return 1;
}

int http_match_verb(const struct http_string* method, const char* verb){
    M_REQUIRE_NON_NULL(method);
    M_REQUIRE_NON_NULL(verb);

    if (method->len != strlen(verb)) {
        return 0;
    }

    for (size_t i = 0; i < strlen(verb); ++i) {
        if (method->val[i] != verb[i]) {
            return 0;
        }
    }
    return 1;
}

int http_get_var(const struct http_string* url, const char* name, char* out, size_t out_len){
    M_REQUIRE_NON_NULL(url);
    M_REQUIRE_NON_NULL(name);
    M_REQUIRE_NON_NULL(out);


     char* args_begin = strstr(url->val, "?"); //to check the beginning of the arguments
    if (args_begin == NULL) {
        return ERR_INVALID_ARGUMENT;
    }

    char* name_cpy = calloc(strlen(name) + 2, sizeof(char));
    if (name_cpy == NULL) {
        return ERR_IO;//todo check if this is the right error
    }

    strcpy(name_cpy, name);
    strcat(name_cpy, "=");

    char* start = strstr(args_begin, name_cpy);
    if (start == NULL) {
        free(name_cpy);
        return 0;
    }

    char* end = strstr(start, "&");
    if (end == NULL) {
        end = url->val + url->len;
    }
    start += strlen(name_cpy);
    size_t arg_length = end - start;

    if (arg_length > out_len || arg_length <= 0) {//todo check if this should not be >= in case we have dont have to add a null terminator
        free(name_cpy);
        return ERR_RUNTIME;
    }
    char arg[arg_length + 1];
    memcpy(arg, start, arg_length);

    arg[arg_length] = '\0';
    memcpy(out, arg, arg_length + 1);

    free(name_cpy);
    return arg_length;
}

static const char* get_next_token(const char* message,
                                  const char* delimiter,
                                  struct http_string* output){
    // find the position of the delimiter
    const char* end_token = strstr(message, delimiter);
    if (end_token == NULL) {
        // if not found return 
        return NULL;//todo check if correct return
    }
    
    if (output != NULL) {
        output->val = message;          // pointer to the beginning of the token
        output->len = end_token - message;    // length of the substring
    }
    // point just after the delimiter
    return end_token + strlen(delimiter);
}

static const char* http_parse_headers(const char* header_start,
                                      struct http_message* output) {

    const char* current = get_next_token(header_start,
                                         HTTP_LINE_DELIM,
                                          NULL);
    struct http_string line;

    output -> num_headers = 0;

    while ((current = get_next_token(current, HTTP_HDR_KV_DELIM, &line)))
    {//todo didn't use HTTP_HDR_END_DELIM for the end of the headers hope that way works
        if(line.len <= 0) break;

        struct http_header current_header;
        current_header.key = line;

        if (!(current = get_next_token(current, HTTP_LINE_DELIM, &line))) {
            return NULL;
        }//todo useless check but why not

        current_header.value = line;

        output -> num_headers++;

        if (output -> num_headers >= MAX_HEADERS ) {
            return NULL; //todo not doing any error just stoping looking for headers
        }

        output -> headers[output -> num_headers] = current_header;

    }

    return current; // Retourne la position après les en-têtes
}







int http_parse_message(const char *stream,
                       size_t bytes_received,
                       struct http_message *out,
                       int *content_len) {

    M_REQUIRE_NON_NULL(stream);
    M_REQUIRE_NON_NULL(out);
    M_REQUIRE_NON_NULL(content_len);

    if(bytes_received == 0){
        return ERR_INVALID_ARGUMENT;//todo check if this is the right error
    }

}


int main() {
    const char* headers = "Host: localhost:8000\r\nUser-Agent: curl/8.5.0\r\nAccept: */*\r\n\r\n";
    struct http_message message;

    const char* body_start = http_parse_headers(headers, &message);
    if (body_start) {
        for (size_t i = 0; i < message.header_count; i++) {
            printf("Header %zu:\n", i);
            printf("  Key: %.*s\n", (int)message.headers[i].key.len, message.headers[i].key.val);
            printf("  Value: %.*s\n", (int)message.headers[i].value.len, message.headers[i].value.val);
        }
    } else {
        printf("Error parsing headers.\n");
    }

    return 0;
}