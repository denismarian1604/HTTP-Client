#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> 
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

// Function used to compute a GET request
char *compute_get_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count,
                            char **tokens, int tokens_count)
{
    char *message = calloc(BUFLEN, sizeof(char));
    DIE(message == NULL, "[Error] allocating memory for message");
    char *line = calloc(LINELEN, sizeof(char));
    DIE(line == NULL, "[Error] allocating memory for line");

    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Add the cookies
    if (cookies != NULL) {
        memset(line, 0, LINELEN);

        char *cookie_data_buffer = calloc(LINELEN, sizeof(char));
        DIE(cookie_data_buffer == NULL, "[Error] allocating memory for cookie data buffer");
        sprintf(cookie_data_buffer, "Cookie: %s", cookies[0]);

        for (int i = 1; i < cookies_count; i++) {
            strcat(cookie_data_buffer, "; ");
            strcat(cookie_data_buffer, cookies[i]);
        }

        compute_message(message, cookie_data_buffer);
    }

    // Add the tokens
    if (tokens != NULL) {
        memset(line, 0, LINELEN);

        char *token_data_buffer = calloc(LINELEN, sizeof(char));
        DIE(token_data_buffer == NULL, "[Error] allocating memory for token data buffer");
        sprintf(token_data_buffer, "Authorization: Bearer %s", tokens[0]);

        for (int i = 1; i < tokens_count; i++) {
            strcat(token_data_buffer, "; ");
            strcat(token_data_buffer, tokens[i]);
        }

        compute_message(message, token_data_buffer);
    }


    compute_message(message, "");
    return message;
}

// Function used to compute a DELETE request
char *compute_delete_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count,
                            char **tokens, int tokens_count)
{
    char *message = calloc(BUFLEN, sizeof(char));
    DIE(message == NULL, "[Error] allocating memory for message");
    char *line = calloc(LINELEN, sizeof(char));
    DIE(line == NULL, "[Error] allocating memory for line");

    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }

    compute_message(message, line);

    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Add the cookies
    if (cookies != NULL) {
        memset(line, 0, LINELEN);

        char *cookie_data_buffer = calloc(LINELEN, sizeof(char));
        DIE(cookie_data_buffer == NULL, "[Error] allocating memory for cookie data buffer");
        sprintf(cookie_data_buffer, "Cookie: %s", cookies[0]);

        for (int i = 1; i < cookies_count; i++) {
            strcat(cookie_data_buffer, "; ");
            strcat(cookie_data_buffer, cookies[i]);
        }

        compute_message(message, cookie_data_buffer);
    }

    // Add the tokens
    if (tokens != NULL) {
        memset(line, 0, LINELEN);

        char *token_data_buffer = calloc(LINELEN, sizeof(char));
        DIE(token_data_buffer == NULL, "[Error] allocating memory for token data buffer");
        sprintf(token_data_buffer, "Authorization: Bearer %s", tokens[0]);

        for (int i = 1; i < tokens_count; i++) {
            strcat(token_data_buffer, "; ");
            strcat(token_data_buffer, tokens[i]);
        }

        compute_message(message, token_data_buffer);
    }

    compute_message(message, "");
    return message;
}

// Function used to compute a POST requests
char *compute_post_request(char *host, char *url, char* content_type, char **body_data,
                            int body_data_fields_count,
                            char **cookies, int cookies_count,
                            char **tokens, int tokens_count)
{
    char *message = calloc(BUFLEN, sizeof(char));
    DIE(message == NULL, "[Error] allocating memory for message");
    char *line = calloc(LINELEN, sizeof(char));
    DIE(line == NULL, "[Error] allocating memory for line");
    char *body_data_buffer = calloc(LINELEN, sizeof(char));
    DIE(body_data_buffer == NULL, "[Error] allocating memory for body data buffer");

    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    int content_length = 0;
    memset(line, 0, LINELEN);
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    for (int i = 0; i < body_data_fields_count; i++) {
        content_length += strlen(body_data[i]);
        strcat(body_data_buffer, body_data[i]);
    }
    memset(line, 0, LINELEN);
    sprintf(line, "Content-Length: %d", content_length);
    compute_message(message, line);

    // Add the cookies
    if (cookies != NULL) {
        memset(line, 0, LINELEN);

        char *cookie_data_buffer = calloc(LINELEN, sizeof(char));
        DIE(cookie_data_buffer == NULL, "[Error] allocating memory for cookie data buffer");
        sprintf(cookie_data_buffer, "Cookie: %s", cookies[0]);

        for (int i = 1; i < cookies_count; i++) {
            strcat(cookie_data_buffer, "; ");
            strcat(cookie_data_buffer, cookies[i]);
        }

        compute_message(message, cookie_data_buffer);
    }

    // Add the tokens
    if (tokens != NULL) {
        memset(line, 0, LINELEN);

        char *token_data_buffer = calloc(LINELEN, sizeof(char));
        DIE(token_data_buffer == NULL, "[Error] allocating memory for token data buffer");
        sprintf(token_data_buffer, "Authorization: Bearer %s", tokens[0]);

        for (int i = 1; i < tokens_count; i++) {
            strcat(token_data_buffer, "; ");
            strcat(token_data_buffer, tokens[i]);
        }

        compute_message(message, token_data_buffer);
    }

    compute_message(message, "");

    memset(line, 0, LINELEN);
    strcat(message, body_data_buffer);

    free(line);
    return message;
}
