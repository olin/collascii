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

    sendall(sockfd, "foo", 3);

    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';

    printf("received:\n%s\n",buf);

    close(sockfd);

    return 0;
}