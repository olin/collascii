#include <stdlib.h>
#include <stdio.h>

#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

#include "network.h"

/* Wrapper for send() that sends the entire buffer
 *
 * Calls it multiple times if necessary.
 *
 * Returns the number of bytes sent or -1 on error
 */
int _send(int sockfd, char *buffer, int bufflen)
{
    int numsent = 0; // total number of bytes sent
    int res = 0;     // saved for return value of send
    while (numsent < bufflen)
    {
        res = send(sockfd, buffer + numsent, bufflen - numsent, 0);
        if (res == -1)
        {
            perror("_send");
            return -1;
        }
        numsent += res;
    }
    return numsent;
}

/* An IPv4 and IPv6 agnostic way to get an in_addr type
 *
 * Returns: a pointer to either a in_addr or in6_addr type
 */
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

/* Get a string representation of the IP of an addrinfo.
 *
 * Returns: -1 on error, 0 otherwise
 */
int get_addrinfo_ip(struct addrinfo *p, char *s, int slen)
{
    if (inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
                  s, slen) == NULL)
    {
        perror("inet_ntop");
        return -1;
    }
    return 0;
}

/* Get the first available socket from the addrinfo linked list.
 *
 * Caller must free the addrinfo that list is pointed towards (this frees res as well).
 *
 * Returns: a socket file descriptor, or -1 on error
 *
 * TODO: accept function to test sockets
 * TODO: find a cleaner way of returning addresses
 */
int get_socket(char *host, char *port, struct addrinfo *hints, struct addrinfo **res, struct addrinfo **list)
{
    // get addrinfo linked list
    int returncode;
    // struct addrinfo *infolist;
    if ((returncode = getaddrinfo(host, port, hints, list)) != 0)
    {
        // getaddrinfo doesn't use errno, except for special cases
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(returncode));
        return -1;
    }

    // loop through all the results and find a working socket
    int sockfd;
    struct addrinfo *p;
    for (p = *list; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            perror("socket");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("connect");
            continue;
        }
        break;
    }
    if (p == NULL)
    {
        return -1;
    }
    *res = p;
    return sockfd;
}