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

    char* name_cpy = calloc(strlen(name) + 2, sizeof(char));
    if (name_cpy == NULL) {
        return ERR_IO;
    }
    strcpy(name_cpy, name);
    strcat(name_cpy, "=");

    char* start = strstr(url->val, name_cpy);
    if (start == NULL) {
        free(name_cpy);
        return 0;
    }

    char* end = strstr(start, "&");
    if (end == NULL) {
        end = url->val + url->len;
    }
    size_t arg_length = end - start + strlen(name_cpy);

    if (arg_length > out_len) {
        free(name_cpy);
        return ERR_RUNTIME;
    }
    char arg[arg_length + 1];
    memcpy(arg, start + strlen(name_cpy), arg_length);

    arg[arg_length] = '\0';
    memcpy(out, arg, arg_length + 1);

    free(name_cpy);
    return arg_length;
}


int http_parse_message(const char *stream, size_t bytes_received, struct http_message *out, int *content_len) {
    M_REQUIRE_NON_NULL(stream);
    M_REQUIRE_NON_NULL(out);
    M_REQUIRE_NON_NULL(content_len);

    if(bytes_received == 0){
        return ERR_INVALID_ARGUMENT;//todo check if this is the right error
    }
}