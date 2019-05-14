#ifndef frontend_h
#define frontend_h

#include <ncurses.h>
#include "cursor.h"
#include "mode_id.h"
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

typedef struct {
  WINDOW *status_win, *info_win, *msg_win, *mode_win;
} status_interface_t;

status_interface_t *create_status_interface();
void destroy_status_interface(status_interface_t *si);
void destroy_win();

int print_msg_win(char *format, ...);
int print_info_win(char *format, ...);
int print_mode_win(char *format, ...);
void update_info_win(const Mode_ID current_mode, const int x, const int y,
                     const int w, const int h);
#endif
