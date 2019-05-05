/*

    Server for CollASCII

    mostly copied from:
   https://github.com/yorickdewid/Chat-Server/blob/master/chat_server.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>

// #include "network.h"
#include "canvas.h"

static _Atomic unsigned int cli_count = 0;
static int uid = 10;

#define MAX_CLIENTS 100
#define BUFFER_SZ 2048

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

/* Add client to queue */
void queue_add(client_t *cl) {
  pthread_mutex_lock(&clients_mutex);
  for (int i = 0; i < MAX_CLIENTS; ++i) {
    if (!clients[i]) {
      clients[i] = cl;
      break;
    }
  }
  pthread_mutex_unlock(&clients_mutex);
}

/* Delete client from queue */
void queue_delete(int uid) {
  pthread_mutex_lock(&clients_mutex);
  for (int i = 0; i < MAX_CLIENTS; ++i) {
    if (clients[i]) {
      if (clients[i]->uid == uid) {
        clients[i] = NULL;
        break;
      }
    }
  }
  pthread_mutex_unlock(&clients_mutex);
}

/* Send message to all clients but the sender */
void send_message(char *s, int uid) {
  pthread_mutex_lock(&clients_mutex);
  for (int i = 0; i < MAX_CLIENTS; ++i) {
    if (clients[i]) {
      if (clients[i]->uid != uid) {
        if (write(clients[i]->connfd, s, strlen(s)) < 0) {
          perror("Write to descriptor failed");
          break;
        }
      }
    }
  }
  pthread_mutex_unlock(&clients_mutex);
}

/* Send message to sender */
void send_message_self(const char *s, int connfd) {
  if (write(connfd, s, strlen(s)) < 0) {
    perror("Write to descriptor failed");
    exit(-1);
  }
}

/* Send message to client */
void send_message_client(char *s, int uid) {
  pthread_mutex_lock(&clients_mutex);
  for (int i = 0; i < MAX_CLIENTS; ++i) {
    if (clients[i]) {
      if (clients[i]->uid == uid) {
        if (write(clients[i]->connfd, s, strlen(s)) < 0) {
          perror("Write to descriptor failed");
          break;
        }
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

  sprintf(buff_out, "/canvas_size %d %d\n", canvas->num_cols, canvas->num_rows);
  send_message_self(buff_out, cli->connfd);
  printf("sent canvas size\n");
  canvas_serialize(canvas, canvas_buf);
  send_message_self(canvas_buf, cli->connfd);
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

    /* Special options */
    if (buff_in[0] == '/') {
      char c = buff_in[strlen(buff_in) - 1];
      char *command;
      command = strtok(buff_in, " ");
      if (!strcmp(command, "/quit") || !strcmp(command, "/q")) {
        break;
      }
      if (!strcmp(command, "/set")) {
        int x = atoi(strtok(NULL, " "));
        int y = atoi(strtok(NULL, " "));

        if (x > canvas->num_cols || y > canvas->num_rows) {
          printf("set out of bounds: (%d,%d)\n", x, y);
        } else {
          printf("setting (%d,%d) to '%c'\n", x, y, c);
          canvas_scharyx(canvas, x, y, c);

          sprintf(buff_out, "/set %d %d %c", x, y, c);
          send_message(buff_out, cli->uid);
        }
      } else if (!strcmp(command, "/canvas")) {
        canvas_serialize(canvas, canvas_buf);
        send_message_self(canvas_buf, cli->connfd);
      }
    } else {
      /* Send message */
      snprintf(buff_out, sizeof(buff_out), "[%s] %s\r\n", cli->name, buff_in);
      send_message(buff_out, cli->uid);
      printf("%s\n", buff_out);
    }
  }

  /* Close connection */
  close(cli->connfd);

  /* Delete client from queue and yield thread */
  queue_delete(cli->uid);
  printf("<< quit ");
  print_client_addr(cli->addr);
  printf(" referenced by %d\n", cli->uid);
  free(cli);
  cli_count--;
  pthread_detach(pthread_self());

  return NULL;
}

/* listenandloop
 *
 * Blocks and listens on a port for connections
 *
 * TODO: accept a handler function
 * TODO: add a way to break out of the function
 */
/*
int listenandloop(char *port)
{
    int status;
    // get network info
    struct addrinfo hints, *res, *p;

    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // use IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    if ((status = getaddrinfo(NULL, port, &hints, &res)) != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    // res now points to a linked list of 1 or more struct addrinfos

    // passive socket to listen on
    int sockfd;

    // loop through all the results and bind to the first available
    int yes = 1;
    for (p = res; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            perror("socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1)
        {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("bind");
            continue;
        }

        break;
    }

    freeaddrinfo(res); // free the linked-list

    if (p == NULL)
    {
        fprintf(stderr, "failed to bind\n");
        exit(1);
    }

    // listen on the port
    // TODO: pick a better number for backlog
    if (listen(sockfd, 5) == -1)
    {
        perror("listen");
        exit(1);
    }
    printf("Listening on port %s...\n", port);

    while (1)
    {
        // accept incoming connection (blocking?)
        struct sockaddr_storage their_addr;
        socklen_t addr_size;
        int incoming_fd;
        addr_size = sizeof(their_addr);
        incoming_fd = accept(sockfd, (struct sockaddr *)&their_addr,
&addr_size); if (incoming_fd == -1)
        {
            perror("accept");
            exit(1);
        }
        printf("New connection\n");

        while (handlerequest(incoming_fd) == 0)
        {
            printf("Handled request.\n\n");
        }

        close(incoming_fd);
        printf("Closed connection\n");
        // break;
    }
    return 0;
}
*/

int main() {
  canvas = canvas_new_blank(20, 20);
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
  printf("connected to port %d", port);

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
