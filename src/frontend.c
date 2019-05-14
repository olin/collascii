/*
 *   ___ ___  _    _      _   ___  ___ ___ ___
 *  / __/ _ \| |  | |    /_\ / __|/ __|_ _|_ _|
 * | (_| (_) | |__| |__ / _ \\__ \ (__ | | | |
 *  \___\___/|____|____/_/ \_\___/\___|___|___|
 *
 *   Yesterday's future, tomorrow!
 *
 * A collaborative ASCII editor, in your terminal.
 *
 * By Matthew Beaudouin, Evan New-Schmidt, Adam Novotny
 *
 * +---------------------------------------+ canvas window:
 * | 0 -- X, COLS                          |
 * | |                                     |
 * | Y, ROWS                               |
 * |                                       |
 * |                                       |
 * |                                       |
 * |                                       |
 * |                                       |
 * |                                       |
 * +---------------------------------------+ status window:
 * |Saved to file "art.txt"   [INSERT](2,6)| Message               Info
 * |step: ON, TRANSPARENT: OFF             | Mode
 * +---------------------------------------+
 */

#include "frontend.h"

#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "canvas.h"
#include "cursor.h"
#include "fe_modes.h"
#include "mode_id.h"
#include "network.h"
#include "state.h"
#include "util.h"

WINDOW *canvas_win;
status_interface_t *status_interface;
Cursor *cursor;
View *view;
bool networked = false;

int INFO_WIDTH = 24;  // max width of the info window

char *DEFAULT_FILEPATH = "art.txt";

// signals to be caught with finish() function
int caught_signals[] = {SIGINT,  SIGTERM, SIGKILL, SIGABRT,
                        SIGSEGV, SIGQUIT, SIGSTOP};

// #define LOG_TO_FILE  // redirect stderr to "out.txt"

// mouse movements options (only use one)
// #define ENABLE_MOUSE_MOVEMENT 1003  // all move updates
#define ENABLE_MOUSE_MOVEMENT 1002  // move updates when buttons are pressed
// #define ENABLE_MOUSE_MOVEMENT 0  // don't send any charcodes

#ifdef DEBUG
#define LOG_TO_FILE
#endif

#ifdef LOG_TO_FILE
char *logfile_path = "out.txt";
FILE *logfile = NULL;
#endif

int main(int argc, char *argv[]) {
  // setup finish() signal handler
  for (int i = 0; i < sizeof(caught_signals) / sizeof(int); i++) {
    signal(caught_signals[i], finish);
  }

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
  logd("Starting frontend\n");

  // init canvas from file
  // run before ncurses init so stdin will read correctly
  Canvas *canvas;
  // load canvas from file if argument exists
  char *in_filename = "";
  fd_set testfds;
  int fd;
  Net_cfg *net_cfg;

  if (argc > 1) {
    /* If connecting to server */
    if (!strcmp(argv[1], "-s")) {
      networked = true;
      if (argc == 3) {
        canvas = net_init(argv[2], "");
      } else if (argc == 4) {
        canvas = net_init(argv[2], argv[3]);
      } else {
        printf("Usage:\ncollascii [filename]\ncollascii -s hostname [port]\n");
        exit(1);
      }

      net_cfg = net_getcfg();

      /* If reading from std in */
    } else if (strcmp(argv[1], "-") == 0) {
      // read from stdin if specified
      logd("Reading from stdin\n");
      canvas = canvas_readf_norewind(stdin);
      // reopen stdin b/c EOF has been sent
      // `/dev/tty` points to current terminal
      // note that this is NOT portable
      freopen("/dev/tty", "rw", stdin);

      /* If reading from file */
    } else {
      in_filename = argv[1];
      FILE *f = fopen(in_filename, "r");
      logd("Reading from '%s'\n", in_filename);
      if (f == NULL) {
        printf("cannot read file %s\n", in_filename);
        printf("Usage:\ncollascii [filename]\ncollascii -s hostname [port]\n");

        exit(1);
      }
      canvas = canvas_readf(f);
      fclose(f);
    }
    // canvas_resize(&canvas, 100, 100);
  } else {
    logd("making blank canvas\n");
    canvas = canvas_new_blank(100, 100);
  }

  /* initialize your non-curses data structures here */

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

  // ENABLE MOUSE INPUT
  // grab only mouse movement and left mouse press/release
  mmask_t mmask = BUTTON1_PRESSED | BUTTON1_RELEASED;
#ifdef ENABLE_MOUSE_MOVEMENT
  mmask |= REPORT_MOUSE_POSITION;
#endif
  mmask_t return_mask = mousemask(mmask, NULL);
  logd("Returned mouse mask: %li\n", return_mask);
  // get mouse updates faster at the expense of not registering "clicks"
  mouseinterval(0);
#ifdef ENABLE_MOUSE_MOVEMENT
  // Make the terminal report mouse movement events, in a not-great way.
  // Printing the escape code should bump it into `1003` mode, where an update
  // is sent whenever the mouse moves between cells. Also of note: `1002` only
  // sends movement updates when a mouse button is held down. Supposedly you can
  // configure this in xterm, but I couldn't get it working.
  // The $TERM variable should show some info about what mode is emulated.
  // Running `cat -v` in a shell will show the escaped character codes that are
  // sent and reveal if your shell is configured to send mouse input.
  // https://gist.github.com/sylt/93d3f7b77e7f3a881603
  // https://stackoverflow.com/q/29020638
  // https://stackoverflow.com/q/7462850
  //
  // disabling mouse event charcodes is done in finish()
  if (ENABLE_MOUSE_MOVEMENT == 1002) {
    printf("\033[?1002h\n");
  } else if (ENABLE_MOUSE_MOVEMENT == 1003) {
    printf("\033[?1003h\n");
  }
#endif

  canvas_win = create_canvas_win();
  status_interface = create_status_interface();

  cursor = cursor_new();
  Cursor *last_cursor = cursor_new();

  view = view_new_startpos(canvas, 0, 0);

  // Enable keyboard mapping
  keypad(canvas_win, TRUE);
  keypad(status_interface->status_win, TRUE);

  //// Main loop
  State new_state = {
      .ch_in = OK,
      .cursor = cursor,
      // .current_mode = MODE_PICKER,
      .current_mode = MODE_INSERT,
      // .current_mode = MODE_FREE_LINE,
      // .current_mode = MODE_BRUSH,

      .last_arrow_direction = KEY_RIGHT,
      .last_canvas_mode = MODE_INSERT,
      .view = view,
      .last_cursor = last_cursor,
      .filepath = in_filename[0] == '\0' ? DEFAULT_FILEPATH : in_filename,
  };
  State *state = &new_state;

  // update the screen size first. This clears the status window on any changes
  // (including the first time it's run), so refreshing after updating the
  // status will clear it otherwise
  update_screen_size();

#ifdef DEBUG
  mvwprintw(status_interface->msg_win, 0, 0, "MSG (debug)");
  mvwprintw(status_interface->info_win, 0, 0, "INFO (debug)");
  mvwprintw(status_interface->mode_win, 0, 0, "MODE (debug)");
#endif

  char welcome_msg[] = "COLLASCII: <TAB> to switch modes, <CTRL-C> to quit";
  print_msg_win(welcome_msg);

  // bootstrap initial UI
  call_mode(state->current_mode, START, state);
  update_info_win_state(state);

  // Move cursor to starting location and redraw canvases
  refresh_screen();

  // If connected to server, check both network and keyboard streams
  if (networked) {
    logd("Running networked loop\n");
    while (1) {
      testfds = net_cfg->clientfds;
      select(FD_SETSIZE, &testfds, NULL, NULL, NULL);

      for (fd = 0; fd < FD_SETSIZE; fd++) {
        if (FD_ISSET(fd, &testfds)) {
          if (networked &&
              fd == net_cfg->sockfd) {  // Accept data from open socket
            logd("recv network\n");
            // If server disconnects
            if (net_handler(view) != 0) {
              networked = false;
              print_msg_win("Server Disconnect!");
            };
            redraw_canvas_win();  // TODO: draw single char update
            refresh_screen();
          } else if (fd == 0) {  // process keyboard activity
            master_handler(state, canvas_win, status_interface->info_win);
            refresh_screen();
          }
        }
      }
    }
    // If local, process keyboard stream
  } else {
    logd("Running local loop\n");
    while (1) {
      master_handler(state, canvas_win, status_interface->info_win);
      refresh_screen();
    }
  }
  // Cleanup
  cursor_free(cursor);
  destroy_status_interface(status_interface);
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

/* Update canvas with character at cursor current position.
 *
 * Changes the canvas and updates the ncurses `canvas_win` with the change.
 */
void front_setcharcursor(char ch) {
  canvas_scharyx(view->canvas, cursor->y + view->y, cursor->x + view->x, ch);
  mvwaddch(canvas_win, cursor_y_to_canvas(cursor), cursor_x_to_canvas(cursor),
           ch);
  if (networked) {
    net_send_char(cursor->y + view->y, cursor->x + view->x, ch);
  }
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
      mvwaddch(canvas_win, y + 1, x + 1, ACS_CKBOARD);
    }
  }
  for (int y = max_y; y < view_max_y; y++) {
    for (int x = 0; x < max_x; x++) {
      mvwaddch(canvas_win, y + 1, x + 1, ACS_CKBOARD);
    }
  }
}

void refresh_screen() {
  update_screen_size();
  wmove(canvas_win, cursor_y_to_canvas(cursor), cursor_x_to_canvas(cursor));

  wrefresh(status_interface->msg_win);
  wrefresh(status_interface->info_win);
  wrefresh(status_interface->mode_win);
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

    wclear(stdscr);
    wclear(canvas_win);

    destroy_status_interface(status_interface);
    status_interface = create_status_interface();

    // Redraw borders
    wborder(status_interface->status_win, ACS_VLINE, ACS_VLINE, ACS_HLINE,
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

    redraw_canvas_win();
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

WINDOW *create_msg_win(WINDOW *status_win) {
  int sw = getmaxx(status_win) + 1;
  int x, y;
  getbegyx(status_win, y, x);
  return newwin(1, sw - 2 - INFO_WIDTH, y + 1, x + 1);
}

WINDOW *create_info_win(WINDOW *status_win) {
  int sw = getmaxx(status_win) + 1;
  int x, y;
  getbegyx(status_win, y, x);
  return newwin(1, INFO_WIDTH, y + 1, x + 1 + sw - INFO_WIDTH - 3);
}

WINDOW *create_mode_win(WINDOW *status_win) {
  int sw = getmaxx(status_win) + 1;
  int x, y;
  getbegyx(status_win, y, x);
  return newwin(1, sw - 3, y + 2, x + 1);
}

status_interface_t *create_status_interface() {
  WINDOW *sw = create_status_win();
  status_interface_t *si = malloc(sizeof(status_interface_t));
  *si = (status_interface_t){
      .status_win = sw,
      .msg_win = create_msg_win(sw),
      .info_win = create_info_win(sw),
      .mode_win = create_mode_win(sw),
  };
  wborder(si->status_win, ACS_VLINE, ACS_VLINE, ACS_HLINE,
          ACS_HLINE,  // Sides:   ls,  rs,  ts,  bs,
          ACS_LTEE, ACS_RTEE, ACS_LLCORNER,
          ACS_LRCORNER);  // Corners: tl,  tr,  bl,  br
  return si;
}

void destroy_status_interface(status_interface_t *si) {
  destroy_win(si->msg_win);
  destroy_win(si->info_win);
  destroy_win(si->mode_win);
  destroy_win(si->status_win);
  free(si);
}

void destroy_win(WINDOW *local_win) {
  // Clear borders explicitly
  wborder(local_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');

  wrefresh(local_win);
  delwin(local_win);
}

/* Prints to status_win, similar to printf
 */
int print_msg_win(char *format, ...) {
  // there isn't a va_list version of mvw_printw, so move to status_win first
  // and then use vw_printw
  WINDOW *mw = status_interface->msg_win;
  wclear(mw);
  wmove(mw, 0, 0);
  va_list argp;
  va_start(argp, format);
  int res = vw_printw(mw, format, argp);
  va_end(argp);
  return res;
}

/* Prints to status_win, similar to printf
 */
int print_info_win(char *format, ...) {
  // there isn't a va_list version of mvw_printw, so move to status_win first
  // and then use vw_printw
  WINDOW *mw = status_interface->info_win;
  wclear(mw);
  wmove(mw, 0, 0);
  va_list argp;
  va_start(argp, format);
  int res = vw_printw(mw, format, argp);
  va_end(argp);
  return res;
}

/* Prints to status_win, similar to printf
 */
int print_mode_win(char *format, ...) {
  // there isn't a va_list version of mvw_printw, so move to status_win first
  // and then use vw_printw
  WINDOW *mw = status_interface->mode_win;
  wclear(mw);
  wmove(mw, 0, 0);
  va_list argp;
  va_start(argp, format);
  int res = vw_printw(mw, format, argp);
  va_end(argp);
  return res;
}

/* Update the info subwindow in status_win with relevant info.
 *
 * Prints the current mode, cursor coordinates, and canvas dimensions
 * right-aligned.
 *
 * "[MODE] (X,Y) WxH"
 */
void update_info_win(const Mode_ID current_mode, const int x, const int y,
                     const int w, const int h) {
  WINDOW *mw = status_interface->info_win;
  char buffer[INFO_WIDTH + 1];
  const char *mode_name = modes[current_mode].name;
  int width = snprintf(buffer, INFO_WIDTH + 1, "[%s](%i,%i)%ix%i", mode_name, x,
                       y, w, h);
  wclear(mw);
  wmove(mw, 0, INFO_WIDTH - width);
  waddnstr(mw, buffer, INFO_WIDTH);
}

/* Signal handler for exiting
 *
 * Turns of ncurses, disables mouse moves commands, closes the logfile.
 *
 * If sig is 0 or SIGINT, exits normally, otherwise prints the signal
 * information to stderr and exits with sig.
 */
void finish(int sig) {
  endwin();
// Disable mouse events charcode
#ifdef ENABLE_MOUSE_MOVEMENT
  if (ENABLE_MOUSE_MOVEMENT == 1002) {
    printf("\033[?1002l\n");
  } else if (ENABLE_MOUSE_MOVEMENT == 1003) {
    printf("\033[?1003l\n");
  }
#endif

  /* do your non-curses wrapup here */
#ifdef LOG_TO_FILE
  if (logfile != NULL) {
    fclose(logfile);
  }
#endif
  // decide how to exit based on signal
  switch (sig) {
    case 0:       // normal
    case SIGINT:  // user CTRL-C
      eprintf("Exiting\n");
      exit(0);
      break;
    default:  // problems
      eprintf("Exited with signal %d (%s)\n", sig, strsignal(sig));
      exit(sig);
      break;
  }
}
