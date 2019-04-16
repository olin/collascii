#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include "network.h"

#define PORT "8080"

#define MAXDATASIZE 100 // max number of bytes we can get at once

Canvas *canvas;

int handle_response(int sockfd, requesttype req) {
    char startlinebuffer[128];
    int numread = readline(sockfd, startlinebuffer, sizeof(startlinebuffer));
    printf("handle_response: read %d bytes\n", numread);
    if (numread == sizeof(startlinebuffer)) {
        fprintf(stderr, "handle_response: startline buffer filled\n");
        return -1;
    }
    printf("handle_response: startline: '%s'\n", startlinebuffer);

    char* code = strtok(startlinebuffer, " ");
    char* rest = strtok(NULL, "");
    printf("code: %s\n", code);
    printf("rest: %s\n", rest);
    if (code[0] != '2') {
        printf("Didn't receive a positive code\n");
        return -1;
    }

    // read expected empty line
    char buf[2];
    if ((numread = readline(sockfd, buf, sizeof(buf))) != 0) {
        fprintf(stderr, "handle_response: extra response info, %d bytes\n", numread);
        return -1;
    }

    switch (req)
    {
        case GET:
            // // read size and initialize canvas
            // if (rest == NULL) {
            //     printf("no size returned");
            //     return -1;
            // }
            // Canvas *new_canvas = canvas_new(3, 3);
            // load into canvas
            printf("Reading into canvas.\n");
            return read_canvas(sockfd, canvas);
        case PUT:
            return 0;
        default:
            fprintf(stderr, "Unknown request type\n");
            return -1;
    }

    return 0;
}


int main(int argc, char *argv[])
{
    // TODO:
    // open socket
    // on connect:
    // - GET request
    // - intialize canvas
    // wait for user input
    // on user input:
    // - map canvas change to PUT request
    // - send request
    // - parse response
    // - update canvas accordingly

    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    char s[INET6_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((sockfd = get_socket(argv[1], PORT, &hints, &p, &servinfo)) == -1) {
        fprintf(stderr, "failed to bind\n");
        exit(1);
    }

    get_addrinfo_ip(p, s, sizeof s);
    printf("client: connecting to %s:%s\n", s, PORT);

    freeaddrinfo(servinfo); // all done with this structure

    printf("Connected, getting canvas\n");

    canvas = canvas_new(3, 3);

    request_canvas(sockfd);
    handle_response(sockfd, GET);

    int y, x;
    char c;
    while (1) {
        printf("Canvas:\n");
        canvas_print(canvas);
        printf("enter coordinates and new value (y x char):\n");
        int numread = scanf("%d %d %c", &y, &x, &c);
        if (numread == EOF) {
            perror("scanf");
            exit(0);
        }
        if (numread != 3) {
            printf("Only parsed %d arguments\n", numread);
            exit(1);
        }
        canvas_scharyx(canvas, y, x, c);
        printf("Updated locally.\n");

        printf("Sending to server.\n");
        push_canvas(sockfd, canvas);
        if (handle_response(sockfd, PUT) != 0) {
            exit(1);
        }

        printf("Requesting updated canvas from server.\n");
        request_canvas(sockfd);
        if (handle_response(sockfd, GET) != 0) {
            exit(1);
        }
        printf("\n");
    }

    // sendall(sockfd, "foo", 3);

    // // if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
    // //     perror("recv");
    // //     exit(1);
    // // }

    // buf[numbytes] = '\0';

    // printf("received:\n%s\n",buf);

    close(sockfd);

    return 0;
}