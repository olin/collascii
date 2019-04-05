#include <curses.h>

static void finish(int sig);
static void setup_colors();

int print_status(char* str, WINDOW* window);

WINDOW *create_newwin(int height, int width, int starty, int startx, int should_draw_box);
