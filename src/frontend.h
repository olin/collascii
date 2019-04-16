#ifndef frontend_h
#define frontend_h

#include <ncurses.h>
#include "cursor.h"
#include "view.h"

#define KEY_TAB '\t'
#define KEY_SHIFT_TAB KEY_BTAB
// #define KEY_ENTER '\r' // May be wrong in ncurses

void finish(int sig);
void setup_colors();
void update_screen_size();
void refresh_screen();
void redraw_canvas_win();
void front_scharcursor(char ch);
WINDOW *create_canvas_win();
WINDOW *create_status_win();
void destroy_win();

int print_status(char *str, WINDOW *window);

WINDOW *create_newwin(int height, int width, int starty, int startx,
                      int should_draw_box);

#endif
