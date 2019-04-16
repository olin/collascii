#ifndef network_h
#define network_h

#include <sys/socket.h>
#include <netdb.h>

#include "canvas.h"

typedef enum {
    GET,
    PUT,
} requesttype;


int sendall(int sockfd, char* buffer, int bufflen);
void *get_in_addr(struct sockaddr *sa);
int get_addrinfo_ip(struct addrinfo *p, char *s, int slen);
int send_canvas(int sockfd, Canvas* canvas);
int push_canvas(int sockfd, Canvas* canvas);
int read_canvas(int sockfd, Canvas* canvas);
int readline(int sockfd, char* buffer, int bufsize);
int get_socket(char *host, char *port, struct addrinfo *hints, struct addrinfo **res, struct addrinfo **list);

#endif