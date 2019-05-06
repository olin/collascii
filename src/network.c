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

#define DEBUG

#include "canvas.h"
#include "network.h"
#include "util.h"
#include "view.h"

#define DEFAULT_PORT 5000

#ifdef DEBUG
#define LOG_TO_FILE
#endif

#ifdef LOG_TO_FILE
char *logfile_path = "out_net.txt";
FILE *logfile = NULL;
#endif

/* Network Client Variables */
fd_set testfds, clientfds;
char *msg_buf;
unsigned long msg_size;
int port;
int fd;
int sockfd;
int result;
char hostname[50];
struct hostent *hostinfo;
struct sockaddr_in address;
int networking_enabled;

Canvas *net_setup(int argc, char **argv) {
#ifdef LOG_TO_FILE
  logfile = fopen(logfile_path, "a");
  if (logfile == NULL) {
    perror("logfile fopen:");
    exit(1);
  }
  if (-1 == dup2(fileno(logfile), fileno(stderr))) {
    perror("stderr dup2:");
    exit(1);
  }
#endif

  Canvas *canvas;
  networking_enabled = 1;

  // If running in standalone
  if (argc == 1) {
    networking_enabled = 0;
    return canvas_new_blank(1000, 1000);
  }

  if (argc == 3) {
    //   setup_server
    exit(-1);
  }

  if (!strcmp("-p", argv[1])) {
    if (argc == 2) {
      exit(0);
    } else {
      sscanf(argv[2], "%i", &port);
      strcpy(hostname, argv[3]);
    }
  } else {
    port = DEFAULT_PORT;
    strcpy(hostname, argv[1]);
  }

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  hostinfo = gethostbyname(hostname);
  address.sin_addr = *(struct in_addr *)*hostinfo->h_addr_list;
  address.sin_family = AF_INET;
  address.sin_port = htons(port);

  if (connect(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("connecting");
    exit(1);
  }

  FD_ZERO(&clientfds);
  FD_SET(sockfd, &clientfds);
  FD_SET(0, &clientfds);  // stdin

  msg_size = 50;
  msg_buf = malloc(sizeof(char) * msg_size);
  result = read(sockfd, msg_buf, msg_size - 1);
  msg_buf[result] = '\0'; /* Terminate string with null */
  char *command = strtok(msg_buf, " ");
  if (!strcmp(command, "/canvas_size")) {
    int col = atoi(strtok(NULL, " "));
    int row = atoi(strtok(NULL, " "));

    canvas = canvas_new_blank(row, col);
  } else {
    printf("failed to get canvas size\n");
    exit(1);
  }

  logd("reading\n");

  result = 0;
  int total = 0;
  do {
    result = read(sockfd, msg_buf, msg_size);
    total += result;
    logd("read: %d, %s\n", result, msg_buf);
    canvas_deserialize_partial(msg_buf, canvas, result);
  } while (total < (canvas->num_rows * canvas->num_cols));
  logd("done reading\n");

  return canvas;
}

char net_checksocket(View *view) {
  testfds = clientfds;
  select(FD_SETSIZE, &testfds, NULL, NULL, NULL);

  for (fd = 0; fd < FD_SETSIZE; fd++) {
    if (FD_ISSET(fd, &testfds)) {
      if (fd == sockfd) { /*Accept data from open socket */
        // read data from open socket
        result = read(sockfd, msg_buf, msg_size);
        msg_buf[result] = '\0'; /* Terminate string with null */
        char ch = msg_buf[result - 1];
        char *command = strtok(msg_buf, " ");
        if (!strcmp(command, "/set")) {
          int x = atoi(strtok(NULL, " "));
          int y = atoi(strtok(NULL, " "));

          canvas_scharyx(view->canvas, y, x, ch);
        }
        return 1;
      }
    } else if (fd == 0) {
      return getch();
    }
  }

  return 0;
}

void net_recieve_char(View *view) {
  result = read(sockfd, msg_buf, msg_size);
  msg_buf[result] = '\0'; /* Terminate string with null */
  char ch = msg_buf[result - 1];
  char *command = strtok(msg_buf, " ");
  if (!strcmp(command, "/set")) {
    int x = atoi(strtok(NULL, " "));
    int y = atoi(strtok(NULL, " "));

    canvas_scharyx(view->canvas, y, x, ch);
  }
}
/* Returns new Net_fd object with current clientfds and sockfd */
Net_fd *net_getfd() {
  Net_fd *new_fd = malloc(sizeof(Net_fd));

  new_fd->clientfds = clientfds;
  new_fd->sockfd = sockfd;

  return new_fd;
}

void net_send_char(int x, int y, char ch) {
  if (networking_enabled) {
    char send_buf[50];
    sprintf(send_buf, "/set %d %d %c\n", x, y, ch);
    write(sockfd, send_buf, strlen(send_buf));
  }
}