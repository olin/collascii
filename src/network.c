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
int port = 5000;
int fd;
int sockfd;
FILE *sockstream;
int result;
char hostname[50];
struct hostent *hostinfo;
struct sockaddr_in address;
int networking_enabled = 0;

Canvas *net_init(char *in_hostname, char *in_port) {
  Canvas *canvas;

  // Set port and hostname
  if (strcmp(in_port, "")) {
    sscanf(in_port, "%i", &port);
  }
  strcpy(hostname, in_hostname);

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