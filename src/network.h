#ifndef network_h
#define network_h
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>

#include "canvas.h"
#include "view.h"

typedef struct NET_FD {
  fd_set clientfds;
  int sockfd;
} Net_fd;

Canvas *net_setup(int argc, char **argv);
char net_checksocket(View *view);
Net_fd *net_getfd();
void net_send_char(int x, int y, char ch);
void net_recieve_char(View *view);

#endif