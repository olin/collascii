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

int parse_pos_msg(const char *buff, int *y, int *x, int *uid) {
  return sscanf(buff, "p %i %i %i", y, x, uid) == 3;
}

int net_send_pos(int y, int x) {
  char send_buf[32];
  snprintf(send_buf, 32, "p %d %d 0\n", y, x);
  if (write_fd(sockfd, send_buf) < 0) {
    perrorf("net_send_pos: write_fd");
    return -1;
  }
  return 0;
}

int net_update_pos(State *state) {
  logd("Hello\n");
  static int last_x = 0;
  static int last_y = 0;
  bool updated = false;
  const int cur_x = state->view->x + state->cursor->x;
  const int cur_y = state->view->y + state->cursor->y;
  if (cur_x != last_x) {
    last_x = cur_x;
    updated = true;
  }
  if (cur_y != last_y) {
    last_y = cur_y;
    updated = true;
  }
  if (updated) {
    logd("Sending updated pos\n");
    net_send_pos(cur_y, cur_x);
    return 1;
  }
  return 0;
}

collab_t *collab_create(int uid, int y, int x) {
  collab_t *c = malloc(sizeof(collab_t));
  c->uid = uid;
  c->y = y;
  c->x = x;
  return c;
}

void collab_free(collab_t *collab) {
  free(collab);
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

collab_list_t *collab_list_create(int len) {
  collab_list_t *l = malloc(sizeof(collab_list_t));
  l->len = len;
  l->list = malloc(sizeof(collab_t) * len);
  l->num = 0;
  for (int i = 0; i < len; i++) {
    l->list[i] = NULL;
  }
  return l;
}

void collab_list_free(collab_list_t *l) {
  for (int i = 0; i < l->len; i++) {
    collab_free(l->list[i]);
  }
  free(l->list);
  free(l);
}

int collab_list_add(collab_list_t *l, int uid, int y, int x) {
  int i;
  for (i = 0; i < l->len; i++) {
    if (l->list[i] == NULL) {
      l->list[i] = collab_create(uid, y, x);
      l->num++;
      logd("Added new collaborator %i at (%i, %i)\n", uid, y, x);
      break;
    }
  }
  if (i == l->len) {
    logd("Collaborator list full\n");
    return -1;
  }
  return 0;
}

int collab_list_del(collab_list_t *l, int uid) {
  int i;
  for (i = 0; i < l->len; i++) {
    if (l->list[i]->uid == uid) {
      collab_t *c = l->list[i];
      l->list[i] = NULL;
      collab_free(c);
      l->num--;
      break;
    }
  }
  if (i == l->len) {
    logd("Couldn't find uid %i in list\n", uid);
    return -1;
  }
  return 0;
}

int collab_list_upd(collab_list_t *l, int uid, int y, int x) {
  int i;
  for (i = 0; i < l->len; i++) {
    if (l->list[i] != NULL && l->list[i]->uid == uid) {
      l->list[i]->x = x;
      l->list[i]->y = y;
      logd("Updated collaborator %i to (%i, %i)\n", uid, x, y);
      break;
    }
  }
  if (i == l->len) {
    return collab_list_add(l, uid, y, x);
  }
  return 0;
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
  char *msg = strndup(msg_buf, msg_size);

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
  } else if (strcmp(command, "p") == 0) {
    int y, x, uid;
    // logd("msg_buf: '%s'", msg_buf);
    if (!parse_pos_msg(msg, &y, &x, &uid)) {
      perrorf("net_handler: parse_pos_msg");
    }
    collab_list_upd(state->collab_list, uid, y, x);
  }
  free(msg);
  return 0;
}
