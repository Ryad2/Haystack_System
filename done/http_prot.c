#include "http_prot.h"
#include "error.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Matches the URI in the HTTP message with the target URI
int http_match_uri(const struct http_message *message, const char *target_uri){
    M_REQUIRE_NON_NULL(message);
    M_REQUIRE_NON_NULL(target_uri);

    // Check if the message URI length is less than the target URI length
    if (message->uri.len < strlen(target_uri)) {
        return 0;
    }

    // Compare each character of the message URI with the target URI
    for (size_t i = 0; i < strlen(target_uri); ++i) {
        if (message->uri.val[i] != target_uri[i]) {
            return 0;
        }
    }
    return 1;
}

// Matches the HTTP method in the message with the target verb
int http_match_verb(const struct http_string* method, const char* verb){
    M_REQUIRE_NON_NULL(method);
    M_REQUIRE_NON_NULL(verb);

    // Check if the method length matches the verb length
    if (method->len != strlen(verb)) {
        return 0;
    }

    // Compare each character of the method with the verb
    for (size_t i = 0; i < strlen(verb); ++i) {
        if (method->val[i] != verb[i]) {
            return 0;
        }
    }
    return 1;
}

// Extracts a variable from the URL query string
int http_get_var(const struct http_string* url, const char* name, char* out, size_t out_len){
    M_REQUIRE_NON_NULL(url);
    M_REQUIRE_NON_NULL(name);
    M_REQUIRE_NON_NULL(out);

    // Find the beginning of the query string
    char* args_begin = strstr(url->val, "?"); //to check the beginning of the arguments
    if (args_begin == NULL) {
        return ERR_INVALID_ARGUMENT;
    }

    // Allocate memory for the variable name with "=" appended
    char* name_cpy = calloc(1, strlen(name) + 2);
    if (name_cpy == NULL) {
        return ERR_IO;
    }

    strcpy(name_cpy, name);
    strcat(name_cpy, "=");

    // Find the variable in the query string
    char* start = strstr(args_begin, name_cpy);
    if (start == NULL) {
        free(name_cpy);
        return 0;
    }

    // Find the end of the variable value
    char* end = strstr(start, "&");
    if (end == NULL) {
        end = url->val + url->len;
    }
    start += strlen(name_cpy);
    size_t arg_length = end - start;

    // Check if the output buffer is large enough
    if (arg_length > out_len || arg_length <= 0) {
        free(name_cpy);
        return ERR_RUNTIME;
    }

    // Copy the variable value to the output buffer
    char arg[arg_length + 1];
    memcpy(arg, start, arg_length);
    arg[arg_length] = '\0';
    memcpy(out, arg, arg_length + 1);

    free(name_cpy);
    return arg_length;
}

// Extracts the next token from a message based on the delimiter
static const char* get_next_token(const char* message,
                                  const char* delimiter,
                                  struct http_string* output){
    // find the position of the delimiter
    const char* end_token = strstr(message, delimiter);
    if (end_token == NULL) {
        // if not found return NULL
        return NULL;
    }
    
    if (output != NULL) {
        // Set the output token value and length
        output->val = message;
        output->len = end_token - message;
    }

    // Return the position just after the delimiter
    return end_token + strlen(delimiter);
}

// Parses the HTTP headers
static const char* http_parse_headers(const char* header_start,
                                      struct http_message* output) {
    const char* currentLineStart = header_start;
    struct http_string line;

    output -> num_headers = 0;
    int count = 0;

    // Loop to extract headers
    while ((currentLineStart =
            get_next_token(currentLineStart, HTTP_HDR_KV_DELIM, &line))
            != NULL)
    {
    // since HTTP_HDR_END_DELIM is HTTP_HDR_KV_DELIM twice in a row,
    //we can detect it with a .length of 0 instead of searching for it

        if(line.len <= 0) break;
        struct http_header current_header;
        current_header.key = line;

        if ((currentLineStart =
            get_next_token(currentLineStart, HTTP_LINE_DELIM, &line))
            == NULL) {

            return NULL;
        } //shouldnt happen on a well formed message

        current_header.value = line;

        if (count >= MAX_HEADERS ) {
            return NULL; // Exceeded maximum number of headers
        }
        output -> headers[count] = current_header;
        count++;
        output -> num_headers = count;
    }

    // Return the position of the last header
    return output->headers[count-1].key.val;
}


// Parses an HTTP message
int http_parse_message(const char *stream,
                       size_t bytes_received,
                       struct http_message *out,
                       int *content_len) {

    M_REQUIRE_NON_NULL(stream);
    M_REQUIRE_NON_NULL(out);
    M_REQUIRE_NON_NULL(content_len);

    if(bytes_received == 0){
        return ERR_INVALID_ARGUMENT;
    }

    const char *current_pos = stream;
    const char *headers_end;
    struct http_string token;

    // Verify headers are completely received
    headers_end = strstr(current_pos, HTTP_HDR_END_DELIM);
    if (headers_end == NULL) {
        return 0;  // Headers incomplete
    }

    // Verify headers are completely received
    current_pos = get_next_token(current_pos, " ", &out->method);
    if (current_pos == NULL) {
        return ERR_INVALID_ARGUMENT;  //Parsing Error
    }

    // Parsing the URI and checking if it is valid
    current_pos = get_next_token(current_pos, " ", &out->uri);
    if (current_pos == NULL) {
        return ERR_INVALID_ARGUMENT;  //Parsing Error
    }

    // Parse the HTTP version
    current_pos = get_next_token(current_pos, HTTP_LINE_DELIM, &token);
    if (current_pos == NULL || strncmp(token.val, "HTTP/1.1", token.len) != 0) {
        return ERR_INVALID_ARGUMENT;  // Parsing Error
    }

    //parsing all the headers and storing them in the out struct
    current_pos = http_parse_headers(current_pos, out);
    if (current_pos == NULL) {
        return ERR_INVALID_ARGUMENT;  // Parsing headers error
    }


    *content_len = 0;

    for (size_t i = 0; i < out->num_headers; i++) {
        struct http_header *header = &out->headers[i];

        //looking for the argument Content-Length in the headers
        if (strncmp(header->key.val, "Content-Length", header->key.len) == 0 &&
            header->key.len == strlen("Content-Length")) {
            char content_len_str[header->value.len + 1];

            //copying the value of the header in a string
            memcpy(content_len_str, header->value.val, header->value.len);
            content_len_str[header->value.len] = '\0';
            *content_len = atoi(content_len_str);//converting the string to an int
            break;
        }
    }
    // Parsing the body if present
    if (*content_len > 0) {
        size_t headers_size = headers_end + strlen(HTTP_HDR_END_DELIM) - stream;
        if (bytes_received < headers_size + *content_len) {
            return 0;  // Incomplete body
        }
        out->body.val = headers_end + strlen(HTTP_HDR_END_DELIM);
        out->body.len = *content_len;
    } else {
        // no body or empty body
        out->body.val = NULL;
        out->body.len = 0;
    }
    return 1;  // full message have been parsed
}









