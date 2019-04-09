#include <curses.h>

static void finish(int sig);
static void setup_colors();
WINDOW *create_canvas_win();
WINDOW *create_status_win();

int print_status(char* str, WINDOW* window);

WINDOW *create_newwin(int height, int width, int starty, int startx, int should_draw_box);
