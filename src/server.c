/*
 * Server for Collascii
 *
 * mostly copied from:
 * https://github.com/yorickdewid/Chat-Server/blob/master/chat_server.c
 */

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "canvas.h"
#include "network.h"
#include "util.h"

static _Atomic unsigned int cli_count = 0;
static int uid = 1;  // start uids at 1

#define MAX_CLIENTS 100
#define BUFFER_SZ 2048

#define LOG_TRAFFIC

/* Client structure */
typedef struct {
  struct sockaddr_in addr; /* Client remote address */
  int connfd;              /* Connection file descriptor */
  int uid;                 /* Client unique identifier */
  char name[32];           /* Client name */
} client_t;

client_t *clients[MAX_CLIENTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

Canvas *canvas;
char *canvas_buf;

bool client_eq(client_t *a, client_t *b) {
  return a != NULL && b != NULL && a->uid == b->uid;
}

/* Add client to queue */
void queue_add(client_t *cl) {
  pthread_mutex_lock(&clients_mutex);
  int i;
  for (i = 0; i < MAX_CLIENTS; ++i) {
    if (!clients[i]) {
      clients[i] = cl;
      logd("Stored client %d (%s) at index %d\n", cl->uid, cl->name, i);
      break;
    }
  }
  if (i == MAX_CLIENTS) {
    logd("No room for additional clients!");
  }
  pthread_mutex_unlock(&clients_mutex);
}

// int write_fd(int fd, const char *s) {
//   const int len = strlen(s);
//   const int res = write(fd, s, strlen(s));
// #ifdef LOG_TRAFFIC
//   const int errnum = errno;
//   logd("Wrote %d of %d bytes to descriptor %d: '%s'\n", res, len, fd, s);
//   errno = errnum;
// #endif
//   return res;
// }

int write_client(client_t *client, const char *s) {
  int res = write_fd(client->connfd, s);
  if (res < 0) {
    perrorf("Write to client %i (%s) failed", client->uid, client->name);
  }
  return res;
}

/* Delete client from queue */
void queue_delete(client_t *client) {
  pthread_mutex_lock(&clients_mutex);
  int i;
  for (i = 0; i < MAX_CLIENTS; ++i) {
    if (clients[i]) {
      if (client_eq(clients[i], client)) {
        logd("queue_delete: removed client %d (%s) from queue\n",
             clients[i]->uid, clients[i]->name);
        clients[i] = NULL;
        break;
      }
    }
  }
  if (i == MAX_CLIENTS) {
    logd("queue_delete: couldn't find client %i in queue\n", uid);
  }
  pthread_mutex_unlock(&clients_mutex);
}

/* Send message to all clients but the sender */
void send_message(char *s, client_t *client) {
  pthread_mutex_lock(&clients_mutex);
  for (int i = 0; i < MAX_CLIENTS; ++i) {
    if (clients[i]) {
      if (!client_eq(clients[i], client)) {
        write_client(clients[i], s);
      }
    }
  }
  pthread_mutex_unlock(&clients_mutex);
}

/* Send a message to all clients */
void broadcast_message(char *s) {
  pthread_mutex_lock(&clients_mutex);
  for (int i = 0; i < MAX_CLIENTS; ++i) {
    if (clients[i]) {
      write_client(clients[i], s);
    }
  }
  pthread_mutex_unlock(&clients_mutex);
}

/* Send message to sender */
void send_message_self(const char *s, client_t *client) {
  write_client(client, s);
}

/* Send message to client */
void send_message_client(char *s, client_t *client) {
  pthread_mutex_lock(&clients_mutex);
  for (int i = 0; i < MAX_CLIENTS; ++i) {
    if (client_eq(clients[i], client)) {
      if (write(clients[i]->connfd, s, strlen(s)) < 0) {
        perror("Write to descriptor failed");
        break;
      }
    }
  }
  pthread_mutex_unlock(&clients_mutex);
}

/* Strip CRLF */
void strip_newline(char *s) {
  while (*s != '\0') {
    if (*s == '\r' || *s == '\n') {
      *s = '\0';
    }
    s++;
  }
}

/* Print ip address */
void print_client_addr(struct sockaddr_in addr) {
  printf("%d.%d.%d.%d", addr.sin_addr.s_addr & 0xff,
         (addr.sin_addr.s_addr & 0xff00) >> 8,
         (addr.sin_addr.s_addr & 0xff0000) >> 16,
         (addr.sin_addr.s_addr & 0xff000000) >> 24);
}

/* Handle all communication with the client */
void *handle_client(void *arg) {
  char buff_out[BUFFER_SZ];
  char buff_in[BUFFER_SZ / 2];
  int rlen;

  cli_count++;
  client_t *cli = (client_t *)arg;

  printf("<< accept ");
  print_client_addr(cli->addr);
  printf(" referenced by %d\n", cli->uid);

  // protocol negotiation
  if ((rlen = read(cli->connfd, buff_in, sizeof(buff_in) - 1)) < 0) {
    printf("version negotation: error reading from socket\n");
    goto CLIENT_CLOSE;
  }
  buff_in[rlen] = '\0';
  strip_newline(buff_in);
  char* cmd = strtok(buff_in, " ");
  if (cmd == NULL || cmd[0] != 'v') {
    printf("version negotiation: client command not 'v'\n");

    goto CLIENT_CLOSE;
  }
  char* client_version = strtok(NULL, " ");
  if (client_version == NULL) {
    printf("version negotiation: unable to parse client version\n");
    send_message_self("can't parse version\n", cli);
    goto CLIENT_CLOSE;
  }
  if (strcmp(client_version, PROTOCOL_VERSION) != 0) {
    printf("version negotiation: unknown client protocol version: '%s'\n",
           client_version);
    send_message_self("unknown protocol - supported protocol versions: ", cli);
    send_message_self(PROTOCOL_VERSION, cli);
    send_message_self("\n", cli);
    goto CLIENT_CLOSE;
  }
  send_message_self("vok\n", cli);

  // send canvas
  sprintf(buff_out, "cs %d %d\n", canvas->num_rows, canvas->num_cols);
  send_message_self(buff_out, cli);
  printf("sent canvas size\n");
  canvas_serialize(canvas, canvas_buf);
  send_message_self(canvas_buf, cli);
  sprintf(buff_out, "\n");
  send_message_self(buff_out, cli);
  printf("sent serialized canvas\n");

  /* Receive input from client */
  while ((rlen = read(cli->connfd, buff_in, sizeof(buff_in) - 1)) > 0) {
    buff_in[rlen] = '\0';
    buff_out[0] = '\0';
    strip_newline(buff_in);

    /* Ignore empty buffer */
    if (!strlen(buff_in)) {
      continue;
    }

#ifdef LOG_TRAFFIC
    logd("Read %d bytes from client %d (%s): '%s'\n", rlen, cli->uid, cli->name,
         buff_in);
#endif

    /* Process Command */
    char c = buff_in[strlen(buff_in) - 1];
    char *command;
    char *msg = strndup(buff_in, BUFFER_SZ);
    command = strtok(buff_in, " ");
    if (strcmp(command, "q") == 0) {
      break;
    }

    if (strcmp(command, "s") == 0) {
      int y = atoi(strtok(NULL, " "));
      int x = atoi(strtok(NULL, " "));

      if (x > canvas->num_cols || y > canvas->num_rows) {
        printf("set out of bounds: (%d,%d)\n", x, y);
      } else {
        printf("setting (%d,%d) to '%c'\n", x, y, c);
        canvas_scharyx(canvas, y, x, c);

        sprintf(buff_out, "s %d %d %c\n", y, x, c);
        send_message(buff_out, cli);
      }
    } else if (strcmp(command, "c") == 0) {
      canvas_serialize(canvas, canvas_buf);
      send_message_self(canvas_buf, cli);
    } else if (strcmp(command, "p") == 0) {
      // copy pos message and fill in client id
      char send_buff[32];
      int y, x, uid;
      if (!parse_pos_msg(msg, &y, &x, &uid)) {
        logd("Warning: parse_pos_msg didn't get all of them\n");
      }
      logd("Got (%i, %i) from '%s'\n", x, y, msg);
      if (build_pos_msg(send_buff, 32, y, x, cli->uid) >= 32) {
        logd("handle_client: build_pos_msg buffer too small\n");
      }
      // send to other clients
      send_message(send_buff, cli);
    }
    free(msg);
  }
  CLIENT_CLOSE:

  /* Close connection */
  close(cli->connfd);

  /* Delete client from queue and yield thread */
  queue_delete(cli);
  printf("<< quit ");
  print_client_addr(cli->addr);
  printf(" referenced by %d\n", cli->uid);
  free(cli);
  cli_count--;
  pthread_detach(pthread_self());

  return NULL;
}

/* send quit command to all clients */
void finish(int sig) {
  broadcast_message("q\n");
  exit(sig);
}

int main(int argc, char *argv[]) {
  (void)signal(SIGINT, finish); /* arrange interrupts to terminate */

  if (argc > 1) {
    if (strcmp(argv[1], "-") == 0) {
      // read from stdin if specified
      printf("Reading from stdin\n");
      canvas = canvas_readf_norewind(stdin);
      // reopen stdin b/c EOF has been sent
      // `/dev/tty` points to current terminal
      // note that this is NOT portable
      freopen("/dev/tty", "rw", stdin);

      /* If reading from file */
    } else {
      char *in_filename = argv[1];
      FILE *f = fopen(in_filename, "r");
      printf("Reading from '%s'\n", in_filename);
      if (f == NULL) {
        perror("savefile read");
        exit(1);
      }
      canvas = canvas_readf(f);
      fclose(f);
    }
  } else {
    printf("making blank canvas\n");
    canvas = canvas_new_blank(100, 100);
  }

  canvas_buf = malloc((sizeof(char) * canvas->num_cols * canvas->num_rows) + 1);
  canvas_serialize(canvas, canvas_buf);
  canvas_buf[(canvas->num_cols * canvas->num_rows) + 1] = 0;

  int listenfd = 0, connfd = 0;
  struct sockaddr_in serv_addr;
  struct sockaddr_in cli_addr;
  pthread_t tid;

  /* Socket settings */
  int port = 5000;
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(port);

  /* Ignore pipe signals */
  signal(SIGPIPE, SIG_IGN);

  /* Bind */
  while (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("Socket binding failed");
    serv_addr.sin_port = htons(++port);
  }
  printf("Connected to port %d\n", port);

  /* Listen */
  if (listen(listenfd, 10) < 0) {
    perror("Socket listening failed");
    return EXIT_FAILURE;
  }

  printf("<[ SERVER STARTED ]>\n");

  /* Accept clients */
  while (1) {
    socklen_t clilen = sizeof(cli_addr);
    connfd = accept(listenfd, (struct sockaddr *)&cli_addr, &clilen);

    /* Check if max clients is reached */
    if ((cli_count + 1) == MAX_CLIENTS) {
      printf("<< max clients reached\n");
      printf("<< reject ");
      print_client_addr(cli_addr);
      printf("\n");
      close(connfd);
      continue;
    }

    /* Client settings */
    client_t *cli = (client_t *)malloc(sizeof(client_t));
    cli->addr = cli_addr;
    cli->connfd = connfd;
    cli->uid = uid++;
    sprintf(cli->name, "%d", cli->uid);

    /* Add client to the queue and fork thread */
    queue_add(cli);
    pthread_create(&tid, NULL, &handle_client, (void *)cli);

    /* Reduce CPU usage */
    sleep(1);
  }

  return EXIT_SUCCESS;
}
