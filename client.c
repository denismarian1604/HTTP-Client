#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> 
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <poll.h>

#include "helpers.h"
#include "buffer.h"
#include "requests.h"
#include "parson.h"

#define SERVER_IP "34.246.184.49"
#define SERVER_PORT 8080

// Function used to check if a string contains spaces
int check_string_for_spaces(char *str)
{
    return strstr(str, " ") != NULL;
}

// Function used to update the cookies based on a response message
void update_cookies(char ***cookies, char *response, int *cookies_count) {
    // Beginning of the cookie
    int begin_pos = strstr(response, "Set-Cookie: ") - response + 12;
    // End of the cookie
    int end_pos = strstr(response, "; Path=/") - response;

    if (cookies == NULL) {
        (*cookies) = (char **)malloc(1 * sizeof(char *));
        DIE(cookies == NULL, "Error: allocating memory for cookies");
    } else {
        (*cookies) = (char **)realloc((*cookies), (*cookies_count + 1) * sizeof(char *));
        DIE(cookies == NULL, "Error: reallocating memory for cookies");
    }

    (*cookies)[(*cookies_count)++] = (char *)calloc(end_pos - begin_pos, sizeof(char));
    DIE((*cookies)[(*cookies_count) - 1] == NULL, "Error: allocating memory for cookies[cookies_count - 1]");

    memcpy((*cookies)[(*cookies_count) - 1], response + begin_pos, end_pos - begin_pos);
}

// Function used to update the tokens based on a response message
void update_tokens(char ***tokens, char *response, int *tokens_count) {
    // Beginning of the token
    int begin_pos = strstr(response, "token") - response + 8;
    // End of the token
    int end_pos = strstr(response, "}") - response;

    if (tokens == NULL) {
        (*tokens) = (char **)malloc(1 * sizeof(char *));
        DIE(tokens == NULL, "Error: allocating memory for tokens");
    } else {
        (*tokens) = (char **)realloc((*tokens), (*tokens_count + 1) * sizeof(char *));
        DIE(tokens == NULL, "Error: reallocating memory for tokens");
    }

    (*tokens)[(*tokens_count)++] = (char *)calloc(end_pos - begin_pos, sizeof(char));
    DIE((*tokens)[(*tokens_count) - 1] == NULL, "Error: allocating memory for tokens[tokens_count - 1]");

    memcpy((*tokens)[(*tokens_count) - 1], response + begin_pos, end_pos - begin_pos - 1);
}

// Function used to check if a book's credentials are correct or not
int check_book_for_mistakes(char *title, char *author, char *genre,
                            char *publisher, char *page_count) {
    // Check if the number of pages is, indeed, a number
    int num = atoi(page_count);
    if (num == 0 && page_count[0] != '0') {
        return 1;
    }

    // Check if the title, genre, author and publisher are numbers
    if (atoi(title) != 0 || atoi(author) != 0
        || atoi(genre) != 0 || atoi(publisher) != 0) {
        return 1;
    }

    return 0;
}

// Function used for displaying the user's books collection
void display_given_book_or_list_of_books(char *response, int type) {
    switch(type) {
        // Case of a single book
        case 0:
        {
            response = strstr(response, "{");
            // Parse the response
            JSON_Value *root_value = json_parse_string(response);
            char *serialized_string = json_serialize_to_string_pretty(root_value);

            printf("%s\n", serialized_string);
            break;
        }
        // Case of a list of books
        case 1:
        {
            response = strstr(response, "[");
            // Parse the response
            JSON_Value *root_value = json_parse_string(response);
            char *serialized_string = json_serialize_to_string_pretty(root_value);

            printf("%s\n", serialized_string);
            break;
        }
        default:
            printf("Error: : Should not get here.\n");
    }
}

int main() {
    // Variable used to check if the client is connected to the server
    int connected = 0;
    // Variable used to store the current user
    char current_user[120];
    // Variable used to store the cookies
    char **cookies = NULL;
    // Variable used to store the number of cookies
    int cookies_count = 0;
    // Variable used to store the current tokens
    char **tokens = NULL;
    // Variable used to store the number of tokens
    int tokens_count = 0;

    // Define the pollfd structure array containing only the STDIN file descriptor
    // as it is the only one where events are monitored
    struct pollfd fds[1];

    fds[0].fd = 0;
    fds[0].events = POLLIN;
    
    while (1) {
        DIE(poll(fds, 1, -1) < 0, "Error: poll");

        // If the event is triggered by the STDIN file descriptor
        if (fds[0].revents & POLLIN) {
            // Read the initial command
            char cmd[20];
            memset(cmd, 0, 20);

            read(0, cmd, 20);

            if (!strncmp(cmd, "register", 8)) {
                // Check if the user is already connected to an account
                if (connected) {
                    printf("Error: You are already connected to an account, namely %s.\n", current_user);
                    continue;
                }

                char user[120], pass[120];
                memset(user, 0, 120);
                memset(pass, 0, 120);

                // Read the username and password
                write(1, "username=", 9);
                read(0, user, 120);
                write(1, "password=", 9);
                read(0, pass, 120);

                // Check if the username or password contain spaces
                if (check_string_for_spaces(user) || check_string_for_spaces(pass)) {
                    printf("Error: : Username and password must not contain spaces\n");
                    continue;
                }

                // Remove the new line at the end of the username and password
                user[strlen(user) - 1] = '\0';
                pass[strlen(pass) - 1] = '\0';

                // Open a connection with the server
                int conn_fd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
                DIE(conn_fd < 0, "Error: opening connection with the server");

                // Compute the body data for the request
                JSON_Value *root_value = json_value_init_object();
                JSON_Object *root_object = json_value_get_object(root_value);

                json_object_set_string(root_object, "username", user);
                json_object_set_string(root_object, "password", pass);

                char *body_data = json_serialize_to_string_pretty(root_value);

                // Compute the register POST request
                char *message = compute_post_request(SERVER_IP, "/api/v1/tema/auth/register", "application/json",
                                                    &body_data, 1, NULL, 0, NULL, 0);
                
                // Send the register POST request to the server
                send_to_server(conn_fd, message);

                // Receive the response from the server
                char *response = receive_from_server(conn_fd);

                if (strstr(response, "error") != NULL) {
                    printf("Error: The desired username, %s, is already taken.\n", user);
                } else {
                    printf("Success: The user %s was successfully registered.\n", user);
                }

                // Close the connection with the server
                close_connection(conn_fd);
            } else if(!strncmp(cmd, "login", 5)) {
                // Check if the user is already connected to an account
                if (connected) {
                    printf("Error: You are already connected to an account, namely %s.\n", current_user);
                    continue;
                }

                char user[120], pass[120];
                memset(user, 0, 120);
                memset(pass, 0, 120);

                // Read the username and password
                write(1, "username=\n", 9);
                read(0, user, 120);
                write(1, "password=", 9);
                read(0, pass, 120);

                // Check if the username or password contain spaces
                if (check_string_for_spaces(user) || check_string_for_spaces(pass)) {
                    printf("Error: : Username and password must not contain spaces\n");
                    continue;
                }

                // Remove the new line at the end of the username and password
                user[strlen(user) - 1] = '\0';
                pass[strlen(pass) - 1] = '\0';

                // Open a connection with the server
                int conn_fd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
                DIE(conn_fd < 0, "Error: opening connection with the server");

                // Compute the body data for the request
                JSON_Value *root_value = json_value_init_object();
                JSON_Object *root_object = json_value_get_object(root_value);

                json_object_set_string(root_object, "username", user);
                json_object_set_string(root_object, "password", pass);

                char *body_data = json_serialize_to_string_pretty(root_value);

                // Compute the login POST request
                char *message = compute_post_request(SERVER_IP, "/api/v1/tema/auth/login", "application/json",
                                                    &body_data, 1, NULL, 0, NULL, 0);
                
                // Send the login POST request to the server
                send_to_server(conn_fd, message);

                // Receive the response from the server
                char *response = receive_from_server(conn_fd);

                if (strstr(response, "error") != NULL) {
                    printf("Error: The used credentials are wrong. Please check the spelling and try again.\n");
                } else {
                    printf("Success: Successfully connected as %s.\n", user);

                    // Set the connected flag to 1
                    connected = 1;
                    strcpy(current_user, user);

                    // Obtain the cookies for the current user
                    update_cookies(&cookies, response, &cookies_count);
                }

                // Close the connection with the server
                close_connection(conn_fd);
            } else if (!strncmp(cmd, "enter_library", 13)) {
                // Check if the user is already connected to an account
                if (!connected) {
                    printf("Error: You are not connected to an account. Please login first.\n");
                    continue;
                }

                // Open a connection with the server
                int conn_fd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
                DIE(conn_fd < 0, "Error: opening connection with the server");

                // Compute the enter_library GET request
                char *message = compute_get_request(SERVER_IP, "/api/v1/tema/library/access", NULL,
                                                    cookies, cookies_count,
                                                    tokens, tokens_count);

                // Send the enter_library GEt request to the server
                send_to_server(conn_fd, message);

                // Receive the response from the server
                char *response = receive_from_server(conn_fd);

                if (strstr(response, "error") != NULL) {
                    printf("Error: Insufficient rights to access the library.\n");
                } else {
                    printf("Success: Successfully entered the library.\n");

                    // Obtain the token for the current user
                    update_tokens(&tokens, response, &tokens_count);
                }

                // Close the connection with the server
                close_connection(conn_fd);
            } else if (!strncmp(cmd, "get_books", 9)) {
                // Check if the user is already connected to an account
                if (!connected) {
                    printf("Error: You are not connected to an account. Please login first.\n");
                    continue;
                }

                // Check if the user has the rights to access the library
                if (tokens_count == 0) {
                    printf("Error: You do not have the rights to access the library. Please enter the library first.\n");
                    continue;
                }

                // Open a connection with the server
                int conn_fd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
                DIE(conn_fd < 0, "Error: opening connection with the server");

                // Compute the get_books GET request
                char *message = compute_get_request(SERVER_IP, "/api/v1/tema/library/books", NULL,
                                                    cookies, cookies_count,
                                                    tokens, tokens_count);
                
                // Send the get_books GET request to the server
                send_to_server(conn_fd, message);

                // Receive the response from the server
                char *response = receive_from_server(conn_fd);

                if (strstr(response, "error") != NULL) {
                    printf("Error: Insufficient permissions to get the books.\n");
                } else {
                    printf("Success: Successfully gotten the books. Take a look at your collection :\n");

                    display_given_book_or_list_of_books(response, 1);
                }

                // Close the connection with the server
                close_connection(conn_fd);
            } else if (!strncmp(cmd, "get_book", 8)) {
                // Check if the user is already connected to an account
                if (!connected) {
                    printf("Error: You are not connected to an account. Please login first.\n");
                    continue;
                }

                // Check if the user has the rights to access the library
                if (tokens_count == 0) {
                    printf("Error: You do not have the rights to access the library. Please enter the library first.\n");
                    continue;
                }
                char id[120];
                memset(id, 0, 120);

                // Read the username and password
                write(1, "id=", 3);
                read(0, id, 120);

                // Remove the new line at the end of the id
                id[strlen(id) - 1] = '\0';

                // Check if the id is a number
                if (atoi(id) == 0 && id[0] != '0') {
                    printf("Error: The id must be a number.\n");
                    continue;
                }

                // Open a connection with the server
                int conn_fd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
                DIE(conn_fd < 0, "Error: opening connection with the server");

                // Compute the route
                char route[] = "/api/v1/tema/library/books/";
                strcat(route, id);

                // Compute the get_book GET request
                char *message = compute_get_request(SERVER_IP, route, NULL,
                                                    cookies, cookies_count,
                                                    tokens, tokens_count);
                
                // Send the get_book GET request to the server
                send_to_server(conn_fd, message);

                // Receive the response from the server
                char *response = receive_from_server(conn_fd);

                if (strstr(response, "error") != NULL) {
                    printf("Error: Insufficient permissions to get the book or non-existent book.\n");
                } else {
                    printf("Success: Successfully gotten the book. Take a look at it :\n");

                    display_given_book_or_list_of_books(response, 0);
                }

                // Close the connection with the server
                close_connection(conn_fd);
            } else if (!strncmp(cmd, "add_book", 8)) {
                // Check if the user is already connected to an account
                if (!connected) {
                    printf("Error: You are not connected to an account. Please login first.\n");
                    continue;
                }

                // Check if the user has the rights to access the library
                if (tokens_count == 0) {
                    printf("Error: You do not have the rights to bring changes to the library.\n");
                    continue;
                }

                char title[120], author[120], genre[120], publisher[120], page_count[120];
                memset(title, 0, 120);
                memset(author, 0, 120);
                memset(genre, 0, 120);
                memset(publisher, 0, 120);
                memset(page_count, 0, 120);

                // Read the username and password
                write(1, "title=", 6);
                read(0, title, 120);

                write(1, "author=", 7);
                read(0, author, 120);

                write(1, "genre=", 6);
                read(0, genre, 120);

                write(1, "publisher=", 10);
                read(0, publisher, 120);

                write(1, "page_count=", 11);
                read(0, page_count, 120);

                // Remove the new line at the end of the title, author, genre, publisher and page_count
                title[strlen(title) - 1] = '\0';
                author[strlen(author) - 1] = '\0';
                genre[strlen(genre) - 1] = '\0';
                publisher[strlen(publisher) - 1] = '\0';
                page_count[strlen(page_count) - 1] = '\0';

                // Check if the title, genre, author and publisher are numbers
                if (check_book_for_mistakes(title, author, genre, publisher, page_count)) {
                    printf("Error: : The given informations for the book did not match the guidelines. Please try again.\n");
                    continue;
                }

                // Open a connection with the server
                int conn_fd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
                DIE(conn_fd < 0, "Error: opening connection with the server");

                // Compute the body data for the request
                JSON_Value *root_value = json_value_init_object();
                JSON_Object *root_object = json_value_get_object(root_value);

                json_object_set_string(root_object, "title", title);
                json_object_set_string(root_object, "author", author);
                json_object_set_string(root_object, "genre", genre);
                json_object_set_string(root_object, "publisher", publisher);
                json_object_set_number(root_object, "page_count", atoi(page_count));

                char *body_data = json_serialize_to_string_pretty(root_value);

                // Compute the add_book POST request
                char *message = compute_post_request(SERVER_IP, "/api/v1/tema/library/books", "application/json",
                                                                &body_data, 1,
                                                                cookies, cookies_count,
                                                                tokens, tokens_count);

                // Send the add_book POST request to the server
                send_to_server(conn_fd, message);

                // Receive the response from the server
                char *response = receive_from_server(conn_fd);

                if (strstr(response, "error") != NULL) {
                    printf("Error: You do not have the rights to bring changes to the library.\n");
                } else {
                    printf("Success: The book titled %s by %s was successfully added to the library.\n", title, author);
                }

                // Close the connection with the server
                close_connection(conn_fd);
            } else if (!strncmp(cmd, "delete_book", 11)) {
                // Check if the user is already connected to an account
                if (!connected) {
                    printf("Error: You are not connected to an account. Please login first.\n");
                    continue;
                }

                // Check if the user has the rights to access the library
                if (tokens_count == 0) {
                    printf("Error: You do not have the rights to access the library. Please enter the library first.\n");
                    continue;
                }
                char id[120];
                memset(id, 0, 120);

                // Read the username and password
                write(1, "id=", 3);
                read(0, id, 120);

                // Remove the new line at the end of the id
                id[strlen(id) - 1] = '\0';

                // Check if the id is a number
                if (atoi(id) == 0 && id[0] != '0') {
                    printf("Error: The id must be a number.\n");
                    continue;
                }

                // Open a connection with the server
                int conn_fd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
                DIE(conn_fd < 0, "Error: opening connection with the server");

                char route[] = "/api/v1/tema/library/books/";
                strcat(route, id);

                // Compute the DELETE request
                char *message = compute_delete_request(SERVER_IP, route, NULL,
                                                    cookies, cookies_count,
                                                    tokens, tokens_count);
                
                // Send the DELETE request to the server
                send_to_server(conn_fd, message);

                // Receive the response from the server
                char *response = receive_from_server(conn_fd);

                if (strstr(response, "error") != NULL) {
                    printf("Error: Insufficient permissions to delete the book or non-existent book.\n");
                } else {
                    printf("Success: Successfully deleted the book with the ID %s\n", id);
                }

                // Close the connection with the server
                close_connection(conn_fd);
            } else if (!strncmp(cmd, "logout", 6)) {
                // Check if the user is already connected to an account
                if (!connected) {
                    printf("Error: You are not connected to an account. Please login first.\n");
                    continue;
                }

                // Open a connection with the server
                int conn_fd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
                DIE(conn_fd < 0, "Error: opening connection with the server");

                // Compute the logout GET request
                char *message = compute_get_request(SERVER_IP, "/api/v1/tema/auth/logout", NULL,
                                                    cookies, cookies_count,
                                                    tokens, tokens_count);
                
                // Send the logout GET request to the server
                send_to_server(conn_fd, message);

                // Receive the response from the server
                char *response = receive_from_server(conn_fd);

                if (strstr(response, "error") != NULL) {
                    printf("Error: You are not connected to an account. Please login first.\n");
                } else {
                    printf("Success: Successfully disconnected from the account, namely %s.\n", current_user);

                    // Set the connected flag to 0
                    connected = 0;

                    memset(current_user, 0, 120);

                    // Remove the cookies
                    for (int i = 0; i < cookies_count; i++) {
                        free(cookies[i]);
                    }
                    if (cookies) {
                        free(cookies);
                    }
                    cookies = NULL;

                    // Remove the tokens
                    for (int i = 0; i < tokens_count; i++) {
                        free(tokens[i]);
                    }
                    if (tokens) {
                        free(tokens);
                    }
                    tokens = NULL;

                    // Reset the cookies and tokens count
                    cookies_count = 0;
                    tokens_count = 0;
                }

                // Close the connection with the server
                close_connection(conn_fd);
            } else if (!strncmp(cmd, "exit", 4)) {
                // If the user is not connected, just exit the program
                if (!connected)
                    return 0;

                // If the user was connected, first send a logout request
                // Open a connection with the server
                int conn_fd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
                DIE(conn_fd < 0, "Error: opening connection with the server");

                // Compute the logout GET request
                char *message = compute_get_request(SERVER_IP, "/api/v1/tema/auth/logout", NULL,
                                                    cookies, cookies_count,
                                                    tokens, tokens_count);
                
                // Send the logout GET request to the server
                send_to_server(conn_fd, message);
                
                // Set the connected flag to 0
                connected = 0;

                memset(current_user, 0, 120);

                // Remove the cookies
                for (int i = 0; i < cookies_count; i++) {
                    free(cookies[i]);
                }
                if (cookies) {
                    free(cookies);
                }
                cookies = NULL;

                // Remove the tokens
                for (int i = 0; i < tokens_count; i++) {
                    free(tokens[i]);
                }
                if (tokens) {
                    free(tokens);
                }
                tokens = NULL;

                // Reset the cookies and tokens count
                cookies_count = 0;
                tokens_count = 0;

                return 0;
            } else {
                printf("Error: Unknown command.\n");
            }
        }
    }
}