#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/types.h>

#include "canvas.h"
#include "cursor.h"
#include "fe_modes.h"
#include "frontend.h"
#include "mode_id.h"
#include "state.h"

#define DEFAULT_PORT 5000
#define MSG_SIZE 1000 * 1000

WINDOW *canvas_win, *status_win;
Cursor *cursor;
View *view;
State *state;

/* Layout
 * ___________________________________________
 * | 0 -- X, COLS                           | canvas window
 * | |                                      |
 * | Y, LINES                               |
 * |                                        |
 * |                                        |
 * |                                        |
 * |                                        |
 * |                                        |
 * |                                        |
 * |________________________________________|
 * |                                        |  command window
 * |________________________________________|
 *
 */

int main(int argc, char *argv[]) {
  /* initialize your non-curses data structures here */

  (void)signal(SIGINT, finish); /* arrange interrupts to terminate */

  (void)initscr();      /* initialize the curses library */
  keypad(stdscr, TRUE); /* enable keyboard mapping */
  (void)nonl();         /* tell curses not to do NL->CR/NL on output */
  (void)cbreak();       /* take input chars one at a time, no wait for \n */
  (void)noecho();       /* don't print on getch() */
  curs_set(2);

  define_key("\r", KEY_ENTER);  // Bind the <Enter> key properly

  if (has_colors()) {
    setup_colors();
  }

  canvas_win = create_canvas_win();
  status_win = create_status_win();

  cursor = cursor_new();
  Canvas *canvas;

  /* Network Client Variables */
  fd_set readfds, testfds, clientfds;
  char msg_buf[MSG_SIZE + 1];
  int port;
  int fd;
  int sockfd;
  int result;
  char hostname[50];
  struct hostent *hostinfo;
  struct sockaddr_in address;
  int clientid;
  /* Client setup */

  if (!strcmp("-p", argv[1])) {
    if (argc == 2) {
      printf("Invalid parameters.\nUsage: client [-p PORT] HOSTNAME\n");
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

  result = read(sockfd, msg_buf, MSG_SIZE);
  msg_buf[result] = '\0'; /* Terminate string with null */
  char *command = strtok(msg_buf, " ");
  if (!strcmp(command, "/canvas_size")) {
    int col = atoi(strtok(NULL, " "));
    int row = atoi(strtok(NULL, " "));

    canvas = canvas_new_blank(col, row);
    view = view_new_startpos(canvas, 0, 0);
  } else {
    printf("failed to get canvas size\n");
    exit(1);
  }

  result = read(sockfd, msg_buf, MSG_SIZE);
  canvas_deserialize(msg_buf, canvas);

  // Enable keyboard mapping
  keypad(canvas_win, TRUE);
  keypad(status_win, TRUE);

  char test_msg[] = "Test mode";
  print_status(test_msg, status_win);

  // Move cursor to starting location and redraw
  wmove(canvas_win, cursor_y_to_canvas(cursor), cursor_x_to_canvas(cursor));
  wrefresh(status_win);
  wrefresh(canvas_win);  // Refresh Canvas last so it gets the cursor

  //// Main loop
  State new_state = {
      .ch_in = 0,
      .cursor = cursor,
      .current_mode = MODE_INSERT,
      .last_arrow_direction = KEY_RIGHT,
      .last_canvas_mode = MODE_INSERT,
      .view = view,
      .socket = sockfd,
  };
  state = &new_state;

  while (1) {
    testfds = clientfds;
    select(FD_SETSIZE, &testfds, NULL, NULL, NULL);

    for (fd = 0; fd < FD_SETSIZE; fd++) {
      if (FD_ISSET(fd, &testfds)) {
        if (fd == sockfd) { /*Accept data from open socket */
          // printf("client - read\n");

          // read data from open socket
          result = read(sockfd, msg_buf, MSG_SIZE);
          msg_buf[result] = '\0'; /* Terminate string with null */
          char *command = strtok(msg_buf, " ");
          if (!strcmp(command, "/set")) {
            int x = atoi(strtok(NULL, " "));
            char *y_loc = strtok(NULL, " ");
            int y = atoi(y_loc);
            while (y_loc != 0) {
              y_loc++;
            }
            char ch = y_loc[1];
            printf("c: %c %c", *y_loc, y_loc[1]);
            canvas_scharyx(view->canvas, y, x, ch);
          }

          print_status(msg_buf, status_win);
        } else if (fd == 0) { /*process keyboard activity*/
          state->ch_in = wgetch(canvas_win);
          // fprintf(stderr, "(%c, %i)    ", (char)state->ch_in,
          // state->ch_in);

          mode_functions[state->current_mode](state, canvas_win, status_win);
        }
      }
    }
    refresh_screen();
  }

  // Cleanup
  cursor_free(cursor);
  destroy_win(status_win);
  destroy_win(canvas_win);
  finish(0);
}

void setup_colors() {
  start_color();

  // TODO: Use #define to get colors for standard uses
  // Assign color codes
  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  init_pair(3, COLOR_BLUE, COLOR_BLACK);
  init_pair(4, COLOR_CYAN, COLOR_BLACK);
  init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(6, COLOR_YELLOW, COLOR_BLACK);
  init_pair(7, COLOR_BLACK, COLOR_WHITE);
}

void front_setcharcursor(char ch) {
  int y = cursor_y_to_canvas(cursor) - 1 + view->y;
  int x = cursor_x_to_canvas(cursor) - 1 + view->x;
  canvas_scharyx(view->canvas, y, x, ch);

  char send_buf[MSG_SIZE];
  sprintf(send_buf, "/set %d %d %c\n", x, y, ch);
  write(state->socket, send_buf, strlen(send_buf));
}

void redraw_canvas_win() {
  // find max ranges to draw canvas
  int max_x = view_max_x;
  int max_y = view_max_y;

  if (max_x >= view->canvas->num_cols - view->x)
    (max_x = view->canvas->num_cols - view->x);
  if (max_y >= view->canvas->num_rows - view->y)
    (max_y = view->canvas->num_rows - view->y);

  // draw canvas onto window
  for (int x = 0; x < max_x; x++) {
    for (int y = 0; y < max_y; y++) {
      mvwaddch(canvas_win, y + 1, x + 1,
               canvas_gcharyx(view->canvas, y + view->y, x + view->x));
    }
  }

  // draw fill in rest of window
  for (int x = max_x; x < view_max_x; x++) {
    for (int y = 0; y < view_max_y; y++) {
      mvwaddch(canvas_win, y + 1, x + 1, 'X');
    }
  }
  for (int y = max_y; y < view_max_y; y++) {
    for (int x = 0; x < view_max_x; x++) {
      mvwaddch(canvas_win, y + 1, x + 1, 'X');
    }
  }
}

void refresh_screen() {
  update_screen_size();
  redraw_canvas_win();
  wmove(canvas_win, cursor_y_to_canvas(cursor), cursor_x_to_canvas(cursor));

  wrefresh(status_win);
  wrefresh(canvas_win);  // Refresh Canvas last so it gets the cursor
}

void update_screen_size() {
  static int window_h_old, window_w_old;

  int window_h_new, window_w_new;

  getmaxyx(stdscr, window_h_new, window_w_new);

  if (window_h_new != window_h_old || window_w_new != window_w_old) {
    window_h_old = window_h_new;
    window_w_old = window_w_new;

    wresize(canvas_win, window_h_new - (STATUS_HEIGHT + 1), window_w_new);
    wresize(status_win, STATUS_HEIGHT + 2, window_w_new);

    mvwin(status_win, window_h_new - (STATUS_HEIGHT + 2), 0);

    wclear(stdscr);
    wclear(canvas_win);
    wclear(status_win);

    // Redraw borders
    wborder(status_win, ACS_VLINE, ACS_VLINE, ACS_HLINE,
            ACS_HLINE,  // Sides:   ls,  rs,  ts,  bs,
            ACS_LTEE, ACS_RTEE, ACS_LLCORNER,
            ACS_LRCORNER);  // Corners: tl,  tr,  bl,  br
    wborder(canvas_win, ACS_VLINE, ACS_VLINE, ACS_HLINE,
            ACS_HLINE,  // Sides:   ls,  rs,  ts,  bs,
            ACS_ULCORNER, ACS_URCORNER, ACS_LTEE,
            ACS_RTEE);  // Corners: tl,  tr,  bl,  br

    // Move cursor inside the canvas
    if (cursor->x >= view_max_x) {
      cursor->x = view_max_x;
    }
    if (cursor->y >= view_max_y) {
      cursor->y = view_max_y;
    }
  }
}

WINDOW *create_canvas_win() {
  WINDOW *local_win;

  //                                        + 1 due to bottom border
  local_win = newwin(LINES - (STATUS_HEIGHT + 1), COLS, 0, 0);

  wborder(local_win, ACS_VLINE, ACS_VLINE, ACS_HLINE,
          ACS_HLINE,  // Sides:   ls,  rs,  ts,  bs,
          ACS_ULCORNER, ACS_URCORNER, ACS_LTEE,
          ACS_RTEE);  // Corners: tl,  tr,  bl,  br

  wrefresh(local_win);
  return local_win;
}

WINDOW *create_status_win() {
  WINDOW *local_win;
  //                               + 2 due to horizontal borders
  local_win = newwin(STATUS_HEIGHT + 2, COLS, LINES - (STATUS_HEIGHT + 2), 0);

  wborder(local_win, ACS_VLINE, ACS_VLINE, ACS_HLINE,
          ACS_HLINE,  // Sides:   ls,  rs,  ts,  bs,
          ACS_LTEE, ACS_RTEE, ACS_LLCORNER,
          ACS_LRCORNER);  // Corners: tl,  tr,  bl,  br

  wrefresh(local_win);
  return local_win;
}

void destroy_win(WINDOW *local_win) {
  // Clear borders explicitly
  wborder(local_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');

  wrefresh(local_win);
  delwin(local_win);
}

int print_status(char *str, WINDOW *window) {
  // wattrset(window, COLOR_PAIR(7));
  return mvwprintw(window, 1, 1, str);
}

void finish(int sig) {
  endwin();

  /* do your non-curses wrapup here */

  exit(0);
}
