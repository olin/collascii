#ifndef network_h
#define network_h
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/types.h>

#include "canvas.h"
#include "state.h"
#include "view.h"

#define NUM_COLLAB 20

const char *PROTOCOL_VERSION;

typedef struct NET_CFG {
  fd_set clientfds;
  int sockfd;
} Net_cfg;

int write_fd(int fd, const char *s);

Canvas *net_init(char *hostname, char *port);
Net_cfg *net_getcfg();
int net_handler(State *state);
int net_send_char(int y, int x, char ch);
int net_update_pos(State *state);
int parse_pos_msg(const char *buff, int *y, int *x, int *uid);
int build_pos_msg(char *buff, int buff_len, const int y, const int x,
                  const int uid);

collab_list_t *collab_list_create(int len);
void collab_list_free(collab_list_t *l);

#endif
