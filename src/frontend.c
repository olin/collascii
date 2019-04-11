#include <stdlib.h>
#include <signal.h>
#include "frontend.h"
#include "cursor.h"

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
 * */

int STATUS_HEIGHT = 1;  // not including borders

/* State keeps track of relevant variables for mode functions.
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
    
    WINDOW *canvas_win = create_canvas_win();
    WINDOW *status_win = create_status_win();

    // Enable keyboard mapping
    keypad(canvas_win, TRUE);
    keypad(status_win, TRUE);

    Cursor *cursor = cursor_new();

    char test_msg[] = "Test mode";
    print_status(test_msg, status_win);


    // Move cursor to starting location and redraw
    wmove(canvas_win, cursor_y_to_canvas(cursor), cursor_x_to_canvas(cursor));
    wrefresh(status_win);
    wrefresh(canvas_win); // Refresh Canvas last so it gets the cursor
    

    //// Main loop
    State new_state = { .ch_in = 0,
                        .cursor = cursor,
                        .last_arrow_direction = KEY_RIGHT,
                    };
    State *state = &new_state;

    while ((state->ch_in = wgetch(canvas_win))) {

        if ((state->ch_in == KEY_LEFT) || (state->ch_in == KEY_RIGHT) || (state->ch_in == KEY_UP) || (state->ch_in == KEY_DOWN)) {
            cursor_key_to_move(state->ch_in, state->cursor);
            state->last_arrow_direction = state->ch_in;
        } else {
            if (' ' <= state->ch_in && state->ch_in <= '~') {  // check if ch is printable
                mvwaddch(canvas_win, cursor_y_to_canvas(state->cursor), cursor_x_to_canvas(state->cursor), state->ch_in);
                cursor_key_to_move(state->last_arrow_direction, state->cursor);
            } else if (state->ch_in == KEY_BACKSPACE) {
                cursor_key_to_move(cursor_opposite_dir(state->last_arrow_direction), cursor);
                mvwaddch(canvas_win, cursor_y_to_canvas(cursor), cursor_x_to_canvas(cursor), ' ');
            } else if (state->ch_in == KEY_DC) {
                mvwaddch(canvas_win, cursor_y_to_canvas(cursor), cursor_x_to_canvas(cursor), ' ');
            } else {
                // Print non-print characters to bottom left in status_win bar
                mvwaddch(status_win, 1, COLS-3, state->ch_in); 
            }
        }
        // Move UI cursor to the right place
        wmove(canvas_win, cursor_y_to_canvas(state->cursor), cursor_x_to_canvas(state->cursor));

        wrefresh(status_win);
        wrefresh(canvas_win); // Refresh Canvas last so it gets the cursor
    }

    // Cleanup
    cursor_free(cursor);
    destroy_win(status_win);
    destroy_win(canvas_win);
    finish(0);
}

static void setup_colors() {
    start_color();

    // TODO: Use #define to get colors for standard uses
    // Assign color codes
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

WINDOW *create_canvas_win() {
	WINDOW *local_win;
    
    //                                        + 1 due to bottom border
	local_win = newwin(LINES - (STATUS_HEIGHT + 1), COLS, 0, 0);  // height, width, starty, startx

    wborder(local_win, ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE,      // Sides:   ls,  rs,  ts,  bs,
                       ACS_ULCORNER, ACS_URCORNER, ACS_LTEE, ACS_RTEE); // Corners: tl,  tr,  bl,  br
    
    wrefresh(local_win);
	return local_win;
}

WINDOW *create_status_win() {
	WINDOW *local_win;
    //                               + 2 due to horizontal borders
	local_win = newwin(STATUS_HEIGHT + 2, COLS, 
                        LINES - (STATUS_HEIGHT+2), 0);

    wborder(local_win, ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE,      // Sides:   ls,  rs,  ts,  bs,
                       ACS_LTEE, ACS_RTEE, ACS_LLCORNER, ACS_LRCORNER); // Corners: tl,  tr,  bl,  br
    
    wrefresh(local_win);
	return local_win;
}

void destroy_win(WINDOW *local_win)
{	
	// Clear borders explicitly
    wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
	
	wrefresh(local_win);
	delwin(local_win);
}

int print_status(char* str, WINDOW* window) {
    wattrset(window, COLOR_PAIR(7));
    return mvwprintw(window, 1, 1, str);
}

static void finish(int sig) {
    endwin();

    /* do your non-curses wrapup here */

    exit(0);
}
