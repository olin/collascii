#include "fe_modes.h"

#include "frontend.h"
#include "mode_id.h"

/* Frontend Modes
 *
 * This file contains the functions for different modes.
 *
 * IF YOU ADD A FUNCTION, ADD IT TO THE ENUM AND mode_functions!
 * All functions must have the same signature to fit in mode_functions.
 *
 */

int (*mode_functions[])(State *, WINDOW *, WINDOW *) = {
    mode_picker,
    mode_insert,
    mode_pan,
};

//////////////////////////////
// GENERAL HELPER FUNCTIONS //
//////////////////////////////

/* exit_to_status
 *
 * Helper function to exit from a drawing mode to status mode.
 */
Mode_ID return_to_canvas(int input_ch) {  // State?
  if (input_ch == KEY_ENTER) {
    return LAST;
  }
  /* if Enter
   *      return proper canvas mode
   * else
   *      return null equivalent
   */
  return LAST;
}

////////////////////
// MODE FUNCTIONS //
////////////////////

/* mode_status
 *
 * Default mode. Used to choose other modes.
 */
int mode_picker(State *state, WINDOW *canvas_win, WINDOW *status_win) {
  // Mode Switch - Enter to canvas,
  if (state->ch_in == KEY_ENTER) {
    state->current_mode = state->last_canvas_mode;
    return 0;
  }

  // LR Arrows navigation

  return 0;
}

/* mode_insert
 *
 * Move with arrows and insert character with keyboard.
 */
int mode_insert(State *state, WINDOW *canvas_win, WINDOW *status_win) {
  // handle mode changing
  if (state->ch_in == KEY_TAB) {
    // Clean up code
    state->last_canvas_mode = MODE_INSERT;

    state->current_mode = MODE_PICKER;
    return 0;
  }

  // insert mode behavior
  if ((state->ch_in == KEY_LEFT) || (state->ch_in == KEY_RIGHT) ||
      (state->ch_in == KEY_UP) || (state->ch_in == KEY_DOWN)) {
    cursor_key_to_move(state->ch_in, state->cursor, state->view);
    state->last_arrow_direction = state->ch_in;
  } else {
    if (' ' <= state->ch_in &&
        state->ch_in <= '~') {  // check if ch is printable
      front_scharcursor(state->ch_in);
      cursor_key_to_move(state->last_arrow_direction, state->cursor,
                         state->view);
    } else if (state->ch_in == KEY_BACKSPACE) {
      cursor_key_to_move(cursor_opposite_dir(state->last_arrow_direction),
                         state->cursor, state->view);
      front_scharcursor(' ');
    } else if (state->ch_in == KEY_DC) {
      front_scharcursor(' ');
    } else {
      // Print non-print characters to bottom left in status_win bar
      mvwaddch(status_win, 1, COLS - 3, state->ch_in);
    }
  }
  // Move UI cursor to the right place
  wmove(canvas_win, cursor_y_to_canvas(state->cursor),
        cursor_x_to_canvas(state->cursor));

  return 0;
}

/* Mode Pan

 * Pans the View with arrow keys
 */
int mode_pan(State *state, WINDOW *canvas_win, WINDOW *status_win) {
  // handle mode changing
  if (state->ch_in == KEY_TAB) {
    // Clean up code
    state->last_canvas_mode = MODE_PAN;

    state->current_mode = MODE_PICKER;
    return 0;
  }

  if ((state->ch_in == KEY_LEFT) || (state->ch_in == KEY_RIGHT) ||
      (state->ch_in == KEY_UP) || (state->ch_in == KEY_DOWN)) {
    view_pan_ch(state->ch_in, state->view);
  }

  return 0;
}

////////////////////////////
// SPECIFC MODE FUNCTIONS //
////////////////////////////
