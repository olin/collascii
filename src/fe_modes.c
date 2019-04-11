/* Frontend Modes
 *
 * This file contains the functions for different modes. 
 * 
 * IF YOU ADD A FUNCTION, ADD IT TO THE ENUM AND mode_functions! 
 * All functions must have the same signature to fit in mode_functions.
 * 
 */
#include "fe_modes.h"


  //////////////////////
 // HELPER FUNCTIONS //
//////////////////////

/* exit_to_status
 *
 * Helper function to exit from a drawing mode to status mode.
 */
void exit_to_status(State *state) {
    return;
}


  ////////////////////
 // MODE FUNCTIONS //
////////////////////

/* mode_status
 *
 * Default mode. Used to choose other modes.
 */
int mode_status(State *state, WINDOW *canvas_win, WINDOW *status_win) {
    return 0;
}

/* mode_arrow_input
 *
 * Move with arrows and insert character with keyboard.
 */
int mode_arrow_input(State *state, WINDOW *canvas_win, WINDOW *status_win) {

    if ((state->ch_in == KEY_LEFT) || (state->ch_in == KEY_RIGHT) || (state->ch_in == KEY_UP) || (state->ch_in == KEY_DOWN)) {
            cursor_key_to_move(state->ch_in, state->cursor);
            state->last_arrow_direction = state->ch_in;
        } else {
            if (' ' <= state->ch_in && state->ch_in <= '~') {  // check if ch is printable
                mvwaddch(canvas_win, cursor_y_to_canvas(state->cursor), cursor_x_to_canvas(state->cursor), state->ch_in);
                cursor_key_to_move(state->last_arrow_direction, state->cursor);
            } else if (state->ch_in == KEY_BACKSPACE) {
                cursor_key_to_move(cursor_opposite_dir(state->last_arrow_direction), state->cursor);
                mvwaddch(canvas_win, cursor_y_to_canvas(state->cursor), cursor_x_to_canvas(state->cursor), ' ');
            } else if (state->ch_in == KEY_DC) {
                mvwaddch(canvas_win, cursor_y_to_canvas(state->cursor), cursor_x_to_canvas(state->cursor), ' ');
            } else {
                // Print non-print characters to bottom left in status_win bar
                mvwaddch(status_win, 1, COLS-3, state->ch_in); 
            }
        }
        // Move UI cursor to the right place
        wmove(canvas_win, cursor_y_to_canvas(state->cursor), cursor_x_to_canvas(state->cursor));
        
    return 0;
}
