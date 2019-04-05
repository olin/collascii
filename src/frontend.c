#include <stdlib.h>
#include <signal.h>
#include "frontend.h"

int STATUS_HEIGHT = 1;

/* The Cursor struct helps with controls.
 * It also maps the drawing area to the canvas nicely.
 */

typedef struct CURSOR {
    int x;
    int y;
} Cursor;

Cursor *cursor_init() {
    Cursor *cursor = malloc(sizeof(Cursor));
    cursor->x = 0;
    cursor->y = 0; 
    return cursor;
}

void move_up(Cursor* cursor) {
    if (cursor->y == 0) {
        return;
    }
    cursor->y--;
}

void move_down(Cursor* cursor) {
    if (cursor->y == (LINES - 2)) { // Take box lines into account
        return;
    }
    cursor->y++;
}

void move_left(Cursor* cursor) {
    if (cursor->x == 0) {
        return;
    }
    cursor->x--;
}

void move_right(Cursor* cursor) {
    if (cursor->x == (COLS - 2)) { // Take box lines into account
        return;
    }
    cursor->x++;
}

int x_to_canvas(Cursor* cursor) {
    return cursor->x + 1;  // TODO: Don't hardcode border
}

int y_to_canvas(Cursor* cursor) {
    return cursor->y + 1;  // TODO: Don't hardcode border
}

void free_cursor(Cursor* cursor) {
    free(cursor);
}

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
 * ___________________________________________  
 * |                                        |  command window
 * ___________________________________________
 * */

int main(int argc, char *argv[]) {
    /* initialize your non-curses data structures here */

    (void) signal(SIGINT, finish);      /* arrange interrupts to terminate */

    (void) initscr();      /* initialize the curses library */
    keypad(stdscr, TRUE);  /* enable keyboard mapping */
    (void) nonl();         /* tell curses not to do NL->CR/NL on output */
    (void) cbreak();       /* take input chars one at a time, no wait for \n */
    (void) noecho();       /* don't print on getch() */
    curs_set(2);

    if (has_colors()) { 
        setup_colors();
    }

    // Create Canvas
    WINDOW *canvas = create_newwin(LINES - (STATUS_HEIGHT), COLS,// Account for lines
                                    0, 0, TRUE);
    
    // Create Status bar
    WINDOW *status = create_newwin(STATUS_HEIGHT + 2, COLS, 
                                    LINES - (STATUS_HEIGHT), 0,
                                    FALSE);
    keypad(canvas, TRUE);  /* enable keyboard mapping */

    int num = 0;
    Cursor *cursor = cursor_init();

    char test_msg[] = "Test mode";
    print_status(test_msg, status);

    wrefresh(status);

    int ch;
    while ((ch = wgetch(canvas)))
    {
        switch(ch) {	
            case KEY_LEFT:
                // mvwaddch(canvas, 1, 1, 'L');
				move_left(cursor);
				break;
			case KEY_RIGHT:
                // mvwaddch(canvas, 1, 1, 'R');
				move_right(cursor);
				break;
			case KEY_UP:
                // mvwaddch(canvas, 1, 1, 'U');
				move_up(cursor);
				break;
			case KEY_DOWN:
                // mvwaddch(canvas, 1, 1, 'D');
				move_down(cursor);
				break;
            default:
            // waddch(canvas, ch);
            if (' ' <= ch && ch <= '~') {  // check if ch is printable
                mvwaddch(canvas, y_to_canvas(cursor), x_to_canvas(cursor), ch);
            } else {
                mvwaddch(status, 0, COLS-1, 'X');
            }
		}
        wmove(canvas, y_to_canvas(cursor), x_to_canvas(cursor));
        wrefresh(canvas);

        /* process the command keystroke */
    }

    free_cursor(cursor);
    finish(0);               /* we are done */
}

static void setup_colors() {
    start_color();

    /*
     * Simple color assignment, often all we need.  Color pair 0 cannot
     * be redefined.  This example uses the same value for the color
     * pair as for the foreground color, though of course that is not
     * necessary:
     */
    init_pair(1, COLOR_RED,     COLOR_BLACK);
    init_pair(2, COLOR_GREEN,   COLOR_BLACK);
    init_pair(3, COLOR_BLUE,    COLOR_BLACK);
    init_pair(4, COLOR_CYAN,    COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_YELLOW,  COLOR_BLACK);
    init_pair(7, COLOR_BLACK,   COLOR_WHITE);
}

WINDOW *create_newwin(int height, int width, int starty, int startx, int should_draw_box) {
	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);

    if (should_draw_box) {
        box(local_win, 0 , 0);		/* 0, 0 gives default characters 
                                    * for the vertical and horizontal
                                    * lines			*/
        wrefresh(local_win);		/* Show that box 		*/

    }

	return local_win;
}

int print_status(char* str, WINDOW* window) {
    wattrset(window, COLOR_PAIR(7));
    return mvwprintw(window, 0, 1, str);
}

static void finish(int sig) {
    endwin();

    /* do your non-curses wrapup here */

    exit(0);
}
