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
#include "network.h"
#include "util.h"
#include "view.h"

/* Network Client Variables */
fd_set testfds, clientfds;
char *msg_buf;
size_t msg_size;
// char *port = "5000";
int port = 5000;
int fd;
int sockfd;
FILE *sockstream;
int result;
char *hostname;
struct hostent *hostinfo;
struct sockaddr_in address;
struct addrinfo hints, *servinfo;

/* Connects to server and returns a canvas
 *
 */
Canvas *net_init(char *in_hostname, char *in_port) {
  // Canvas *canvas;

  // // Set port if in_port is not blank
  // if (strcmp(in_port, "")) {
  //   port = in_port;
  // }
  // hostname = strdup(in_hostname);

  // memset(&hints, 0, sizeof hints);
  // struct addrinfo hints = {
  //     .ai_family = AF_UNSPEC,
  //     .ai_socktype = SOCK_STREAM,
  // };

  // int returncode;
  // if ((returncode = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
  //   // getaddrinfo doesn't use errno, except for special cases
  //   printf("getaddrinfo: %s\n", gai_strerror(returncode));
  //   exit(1);
  // }

  // // loop through all the results and find a working socket
  // struct addrinfo *p;
  // for (p = servinfo; p != NULL; p = p->ai_next) {
  //   if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) ==
  //   -1) {
  //     perror("socket");
  //     continue;
  //   }
  //   if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
  //     close(sockfd);
  //     perror("connect");
  //     continue;
  //   }
  //   break;
  // }
  // if (p == NULL) {
  //   perror("connecting");
  //   printf("Cannot connect to server.\n");
  //   exit(1);
  // }

  Canvas *canvas;
  // Set port and hostname
  if (strcmp(in_port, "")) {
    logd("setting port to %s\n", in_port);
    sscanf(in_port, "%i", &port);
  }
  hostname = strdup(in_hostname);
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  hostinfo = gethostbyname(hostname);
  address.sin_addr = *(struct in_addr *)*hostinfo->h_addr_list;
  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  if (connect(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    logd("Failed connecting to server\n");
    exit(1);
  }

  FD_ZERO(&clientfds);
  FD_SET(sockfd, &clientfds);
  FD_SET(0, &clientfds);  // stdin

  sockstream = fdopen(sockfd, "r+");

  getline(&msg_buf, &msg_size, sockstream);
  char *command = strtok(msg_buf, " ");
  if (!strcmp(command, "cs")) {
    int row = atoi(strtok(NULL, " "));
    int col = atoi(strtok(NULL, " "));

    canvas = canvas_new_blank(row, col);
  } else {
    logd("failed to get canvas size\n");
    exit(1);
  }

  logd("reading canvas from server\n");

  getline(&msg_buf, &msg_size, sockstream);
  canvas_deserialize(msg_buf, canvas);

  logd("done reading\n");

  return canvas;
}

/* Returns new Net_cfg object with current clientfds and sockfd
 *
 */
Net_cfg *net_getcfg() {
  Net_cfg *config = malloc(sizeof(Net_cfg));

  config->clientfds = clientfds;
  config->sockfd = sockfd;

  return config;
}

void net_handler(View *view) {
  logd("receiving: ");
  getline(&msg_buf, &msg_size, sockstream);
  logd(msg_buf);
  char ch = msg_buf[strlen(msg_buf) - 2];  // -2 for '\n'

  char *command = strtok(msg_buf, " ");
  logd(command);
  if (!strcmp(command, "s")) {
    int y = atoi(strtok(NULL, " "));
    int x = atoi(strtok(NULL, " "));

    canvas_scharyx(view->canvas, y, x, ch);
  }
}

void net_send_char(int y, int x, char ch) {
  char send_buf[50];
  snprintf(send_buf, 50, "s %d %d %c\n", y, x, ch);
  logd(send_buf);
  write(sockfd, send_buf, strlen(send_buf));

  logd("sending: s %d %d %c\n", y, x, ch);
  // DON"T TRUST FPRINTF!!!
  // fprintf(sockstream, "s %d %d %c\n", y, x, ch);
}
