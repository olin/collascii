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
WINDOW *create_status_win();
void destroy_win();

int global_handler(WINDOW *win);
void read_from_file(char *fname);
void write_to_file(char *fname);
void write_to_file_trim(char *fname);

int print_status(char *format, ...);

#endif
