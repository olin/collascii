#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "network.h"



int handlerequest(int sockfd) {
    // receive from connection
    int numbytes;
    char buf[400];  // TODO: pull out to constant
    if ((numbytes = recv(sockfd, buf, 400-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }
    buf[numbytes] = '\0'; // end buffer string
    printf("Received:\n%s\n", buf);
    // return what was sent
    _send(sockfd, buf, numbytes);
    return 0;
}

/* listenandloop
 *
 * Blocks and listens on a port for connections
 *
 * TODO: accept a handler function
 * TODO: add a way to break out of the function
 */
int listenandloop(char* port) {
    int status;
    // get network info
    struct addrinfo hints, *res, *p;

    memset(&hints, 0, sizeof hints);  // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;      // use IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;  // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;      // fill in my IP for me

    if ((status = getaddrinfo(NULL, port, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    // res now points to a linked list of 1 or more struct addrinfos

    // passive socket to listen on
    int sockfd;

    // loop through all the results and bind to the first available
    int yes = 1;
    for(p = res; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("bind");
            continue;
        }

        break;
    }

    freeaddrinfo(res); // free the linked-list

    if (p == NULL)  {
        fprintf(stderr, "failed to bind\n");
        exit(1);
    }

    // listen on the port
    // TODO: pick a better number for backlog
    if (listen(sockfd, 5) == -1) {
        perror("listen");
        exit(1);
    }
    printf("Listening on port %s...\n", port);

    while (1) {
        // accept incoming connection (blocking?)
        struct sockaddr_storage their_addr;
        socklen_t addr_size;
        int incoming_fd;
        addr_size = sizeof(their_addr);
        incoming_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
        if (incoming_fd == -1) {
            perror("accept");
            exit(1);
        }
        printf("New connection\n");

        handlerequest(incoming_fd);

        close(incoming_fd);
        printf("Closed connection\n");
        // break;
    }
    return 0;
}

int main() {
    listenandloop("8080");
}