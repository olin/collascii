#ifndef frontend_h
#define frontend_h

#include "cursor.h"
#include <curses.h>

// Keep this commented. it gets included through frontend.h
// #include "fe_modes.h"

/* State keeps track of changing variables for mode functions.
 * If you add something, don't forget to also add an init before the main loop.
 * 
 * TODO: replace last_arrow with linked list of previous inputs?
 * Make sure its length is capped.
 */
typedef struct {
    int ch_in;
    Cursor *cursor;
    
    int last_arrow_direction;
} State;


static void finish(int sig);
static void setup_colors();
WINDOW *create_canvas_win();
WINDOW *create_status_win();
void destroy_win();

int print_status(char* str, WINDOW* window);

WINDOW *create_newwin(int height, int width, int starty, int startx, int should_draw_box);

#endif
