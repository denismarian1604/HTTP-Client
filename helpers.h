#ifndef _HELPERS_
#define _HELPERS_

#define BUFLEN 4096
#define LINELEN 1000

#define DIE(assertion, call_description)                                       \
  do {                                                                         \
    if (assertion) {                                                           \
      fprintf(stderr, "(%s, %d): ", __FILE__, __LINE__);                       \
      perror(call_description);                                                \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  } while (0)

// Shows the current error
void error(const char *msg);

// Adds a line to a string message
void compute_message(char *message, const char *line);

// Opens a connection with server host_ip on port portno, returns a socket
int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag);

// Closes a server connection on socket sockfd
void close_connection(int sockfd);

// Send a message to a server
void send_to_server(int sockfd, char *message);

// Receives and returns the message from a server
char *receive_from_server(int sockfd);

#endif
