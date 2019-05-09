#ifndef network_h
#define network_h
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/types.h>

#include "canvas.h"

typedef struct NET_CFG {
  fd_set clientfds;
  int sockfd;
} Net_cfg;

Canvas *net_init(char *hostname, char *port);
Net_cfg *net_getcfg();

#endif