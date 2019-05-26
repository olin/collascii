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
#include "state.h"
#include "util.h"
#include "view.h"

#define LOG_TRAFFIC

/* Network Client Variables */
fd_set testfds, clientfds;
char *msg_buf;
size_t msg_size;
int port = 5000;
int fd;
int sockfd;
FILE *sockstream;
int result;
char *hostname;
struct hostent *hostinfo;
struct sockaddr_in address;
struct addrinfo hints, *servinfo;

const char *PROTOCOL_VERSION = "1.0";

int write_fd(int fd, const char *s) {
  const int len = strlen(s);
  const int res = write(fd, s, strlen(s));
#ifdef LOG_TRAFFIC
  const int errnum = errno;
  logd("Wrote %d of %d bytes to descriptor %d: '%s'\n", res, len, fd, s);
  errno = errnum;
#endif
  return res;
}

int build_set_msg(char *buff, int buff_len, const int y, const int x,
                  const char val) {
  return snprintf(buff, buff_len, "s %i %i %c\n", y, x, val);
}

int build_pos_msg(char *buff, int buff_len, const int y, const int x,
                  const int uid) {
  return snprintf(buff, buff_len, "p %i %i %i\n", y, x, uid);
}

int parse_pos_msg(char *buff, int *y, int *x, int *uid) {
  return sscanf(buff, "p %i %i %i", y, x, uid) == 3;
}

/* Connects to server and returns its canvas
 *
 */
Canvas *net_init(char *in_hostname, char *in_port) {
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

  logd("Trying to connect to %s:%i\n", in_hostname, port);

  if (connect(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("Failed connecting to server");
    exit(1);
  }
  logd("Connected to server successfully\n");

  FD_ZERO(&clientfds);
  FD_SET(sockfd, &clientfds);
  FD_SET(0, &clientfds);  // stdin

  sockstream = fdopen(sockfd, "r+");

  // "negotiate" protocol version
  char version_request_msg[16];
  snprintf(version_request_msg, 16, "v %s\n", PROTOCOL_VERSION);
  if (write(sockfd, version_request_msg, strlen(version_request_msg)) < 0) {
    perror("version negotiation: write error");
    exit(1);
  }

  if (getline(&msg_buf, &msg_size, sockstream) == -1) {
    perror("version negotiation: read error");
    exit(1);
  }
  if (!(msg_buf[0] == 'v' && msg_buf[1] == 'o' && msg_buf[2] == 'k')) {
    eprintf("Failed to negotiate protocol version: the server says '%s'\n",
            msg_buf);
    exit(1);
  }

  // receive canvas from server
  getline(&msg_buf, &msg_size, sockstream);
  char *command = strtok(msg_buf, " ");
  if (strcmp(command, "cs") == 0) {
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

/* Reads incoming packets and updates canvas.
 * Need to run redraw_canvas_win() after calling!
 */
int net_handler(State *state) {
  View *view = state->view;

  getline(&msg_buf, &msg_size, sockstream);
#ifdef LOG_TRAFFIC
  logd("received %li bytes: '%s'\n", msg_size, msg_buf);
#endif
  char ch = msg_buf[strlen(msg_buf) - 2];  // -2 for '\n'

  char *command = strtok(msg_buf, " \n");
  logd("\"%s\"", command);
  if (strcmp(command, "q") == 0) {
    logd("closing socket\n");
    close(sockfd);
    return 1;
  }

  if (strcmp(command, "s") == 0) {
    int y = atoi(strtok(NULL, " "));
    int x = atoi(strtok(NULL, " "));

    canvas_scharyx(view->canvas, y, x, ch);
  }
  if (!strcmp(command, "q")) {
    logd("closing socket\n");
    close(sockfd);
    return 1;
  }

  return 0;
}

/* Sends a set char command to the server
 *
 */
int net_send_char(int y, int x, char ch) {
  char send_buf[50];
  snprintf(send_buf, 50, "s %d %d %c\n", y, x, ch);

  if (write_fd(sockfd, send_buf) < 0) {
    perrorf("net_send_char: write_fd");
    return -1;
  }

  // DON"T TRUST FPRINTF!!! It has failed me!
  // fprintf(sockstream, "s %d %d %c\n", y, x, ch);

  return 0;
}
