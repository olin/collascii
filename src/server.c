#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "network.h"
#include "canvas.h"

Canvas* canvas;

int send_response(int sockfd, int code, char* msg) {
    char buffer[128];
    int numwritten = snprintf(buffer, sizeof(buffer), "%d %s\n\n", code, msg);
    if (numwritten >= sizeof(buffer)) {
        fprintf(stderr, "send_response: reached startline buffer limit");
    }
    return sendall(sockfd, buffer, numwritten);
}

int handlerequest(int sockfd) {
    char startlinebuffer[32];
    int numread = readline(sockfd, startlinebuffer, sizeof(startlinebuffer));
    printf("handle_message: Read %d bytes\n", numread);
    printf("'%s'", startlinebuffer);
    if (numread == sizeof(startlinebuffer)) {
        fprintf(stderr, "handle_message: startline buffer filled");
        return -1;
    }
    printf("handle_message: startline: '%s'\n", startlinebuffer);
    // read startline
    requesttype rtype;
    char* method;
    if ((method = strtok(startlinebuffer, ' ')) == NULL) {
        fprintf(stderr, "handle_message: unable to parse method");
        return -1;
    }

    if (strcmp(method, "GET") == 0) {
        rtype = GET;
        readline(sockfd);

    } else if (strcmp(method, "PUT") == 0) {
        rtype = PUT;
        readline(sockfd);
    }

    char buf[2];
    if (readline(sockfd, buf, sizeof(buf)) != 1) {
        fprintf(stderr, "handle_message: unexpected request headers");
    }

    switch (rtype)
    {
        case GET:
            // return canvas data
            send_response(sockfd, 100, "3x3");
            send_canvas(sockfd, canvas);
            printf("Canvas requested.");
            break;

        case PUT:
            // read and load canvas
            if (read_canvas(sockfd, canvas) != 0) {
                fprintf(stderr, "read_canvas returned nonzero");
                send_response(sockfd, 300, "");
                return -1;
            }
            send_response(sockfd, 200, "");
            printf("Updated canvas.");
            print_canvas(canvas);
            break;

        default:
            // else return error
            send_response(sockfd, 300, "");
            printf("Unknown request");
            return -1;
    }
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
    // TODO:
    // open socket and listen at port
    // wait for connection
    // on each connection:
    // - parse request
    // - handle request
    // - return response
    canvas = make_canvas(3, 3);
    load_string(canvas, "X X X X X");
    listenandloop("8080");
    return 0;
}