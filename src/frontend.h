#ifndef frontend_h
#define frontend_h

#include <ncurses.h>
#include "cursor.h"
#include "view.h"

#define KEY_TAB '\t'
#define KEY_SHIFT_TAB KEY_BTAB
// TODO: Understand delete/backspace on mac

void finish(int sig);
void setup_colors();
void update_screen_size();
void refresh_screen();
void redraw_canvas_win();
void front_setcharcursor(char ch);

WINDOW *create_canvas_win();
// WINDOW *create_status_win();
typedef struct {
  WINDOW *status_win, *info_win, *msg_win, *mode_win;
} status_interface_t;

status_interface_t *create_status_interface();
void destroy_win();

// int print_status(char *format, ...);
int print_msg_win(char *format, ...);
int print_mode_win(char *format, ...);

#endif
