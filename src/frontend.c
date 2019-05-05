#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>

#include "canvas.h"
#include "cursor.h"
#include "fe_modes.h"
#include "frontend.h"
#include "mode_id.h"
#include "state.h"
#include "util.h"

#include <stdio.h>
#include <unistd.h>

WINDOW *canvas_win, *status_win;
Cursor *cursor;
View *view;

#ifdef DEBUG
#define LOG_TO_FILE
#endif

#ifdef LOG_TO_FILE
char *logfile_path = "out.txt";
FILE *logfile = NULL;
#endif

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
  Cursor *last_cursor = cursor_new();
  Canvas *canvas = canvas_new_blank(1000, 1000);

  view = view_new_startpos(canvas, 300, 300);

  // Enable keyboard mapping
  keypad(canvas_win, TRUE);
  keypad(status_win, TRUE);

  // update the screen size first. This clears the status window on any changes
  // (including the first time it's run), so refreshing after updating the
  // status will clear it otherwise
  update_screen_size();

  char test_msg[] = "Test mode";
  print_status(test_msg);

  // Move cursor to starting location and redraw canvases
  refresh_screen();

  //// Main loop
  State new_state = {
      .ch_in = 0,
      .cursor = cursor,
      .current_mode = MODE_INSERT,
      // .current_mode = MODE_FREE_LINE,
      // .current_mode = MODE_BRUSH,

      .last_arrow_direction = KEY_RIGHT,
      .last_canvas_mode = MODE_INSERT,
      .view = view,
      .last_cursor = last_cursor,
  };
  State *state = &new_state;

  while ((state->ch_in = wgetch(canvas_win))) {
    // fprintf(stderr, "(%c, %i)    ", (char)state->ch_in, state->ch_in);

    mode_functions[state->current_mode](state, canvas_win, status_win);

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

/* Update canvas with character at cursor current position.
 */
void front_setcharcursor(char ch) {
  canvas_scharyx(view->canvas, cursor_y_to_canvas(cursor) - 1 + view->y,
                 cursor_x_to_canvas(cursor) - 1 + view->x, ch);
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

/* va_list version of mvwprintwf
 */
int vmvwprintwf(WINDOW *window, int y, int x, char *format, va_list argp) {
  char buffer[64];
  vsnprintf(buffer, sizeof(buffer) / sizeof(char), format, argp);
  return mvwprintw(window, y, x, buffer);
}

/* A variation of mvwprintw that accepts format strings like printf
 *
 * Note that buffer is currently 32 chars long.
 */
int mvwprintwf(WINDOW *window, int y, int x, char *format, ...) {
  va_list argp;
  va_start(argp, format);
  int res = vmvwprintwf(window, y, x, format, argp);
  va_end(argp);
  return res;
}

/* Prints to status_win
 *
 */
int print_status(char *format, ...) {
  // wattrset(window, COLOR_PAIR(7));
  va_list argp;
  va_start(argp, format);
  int res = vmvwprintwf(status_win, 1, 1, format, argp);
  va_end(argp);
  return res;
}

void finish(int sig) {
  endwin();

  /* do your non-curses wrapup here */
#ifdef LOG_TO_FILE
  if (logfile != NULL) {
    fclose(logfile);
  }
#endif
  exit(0);
}
