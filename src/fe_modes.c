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
    mode_free_line,
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
      front_setcharcursor(state->ch_in);
      cursor_key_to_move(state->last_arrow_direction, state->cursor,
                         state->view);
    } else if (state->ch_in == KEY_BACKSPACE) {
      cursor_key_to_move(cursor_opposite_dir(state->last_arrow_direction),
                         state->cursor, state->view);
      front_setcharcursor(' ');
    } else if (state->ch_in == KEY_DC) {
      front_setcharcursor(' ');
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

/* mode_free_line
 *
 * Move with arrows and insert character with keyboard.
 */
int mode_free_line(State *state, WINDOW *canvas_win, WINDOW *status_win) {
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
    int current_arrow = state->ch_in;
    int last_arrow = state->last_arrow_direction;

    mvwaddch(canvas_win, cursor_y_to_canvas(state->cursor),
             cursor_x_to_canvas(state->cursor),
             free_line_arrows_to_char(last_arrow, current_arrow));

    cursor_key_to_move(current_arrow, state->cursor, state->view);
    state->last_arrow_direction = state->ch_in;
  }

  // if ((state->ch_in == KEY_LEFT) || (state->ch_in == KEY_RIGHT) ||
  //     (state->ch_in == KEY_UP) || (state->ch_in == KEY_DOWN)) {
  //   switch (state->ch_in) {
  //     case KEY_LEFT:
  //       cursor_move_left(state->cursor);
  //       mvwaddch(canvas_win, cursor_y_to_canvas(state->cursor),
  //                cursor_x_to_canvas(state->cursor), horizontal);
  //       break;
  //     case KEY_RIGHT:
  //       cursor_move_right(state->cursor);
  //       mvwaddch(canvas_win, cursor_y_to_canvas(state->cursor),
  //                cursor_x_to_canvas(state->cursor), horizontal);
  //       break;
  //     case KEY_UP:
  //       cursor_move_up(state->cursor);
  //       mvwaddch(canvas_win, cursor_y_to_canvas(state->cursor),
  //                cursor_x_to_canvas(state->cursor), vertical);
  //       break;
  //     case KEY_DOWN:
  //       cursor_move_down(state->cursor);
  //       mvwaddch(canvas_win, cursor_y_to_canvas(state->cursor),
  //                cursor_x_to_canvas(state->cursor), vertical);
  //       break;
  //   }

  // cursor_key_to_move(state->ch_in, state->cursor);
  // state->last_arrow_direction = state->ch_in;
  // } else {
  //   if (' ' <= state->ch_in &&
  //       state->ch_in <= '~') {  // check if ch is printable
  //     mvwaddch(canvas_win, cursor_y_to_canvas(state->cursor),
  //              cursor_x_to_canvas(state->cursor), state->ch_in);
  //     cursor_key_to_move(state->last_arrow_direction, state->cursor);
  //   } else if (state->ch_in == KEY_BACKSPACE) {
  //     cursor_key_to_move(cursor_opposite_dir(state->last_arrow_direction),
  //                        state->cursor);
  //     mvwaddch(canvas_win, cursor_y_to_canvas(state->cursor),
  //              cursor_x_to_canvas(state->cursor), ' ');
  //   } else if (state->ch_in == KEY_DC) {
  //     mvwaddch(canvas_win, cursor_y_to_canvas(state->cursor),
  //              cursor_x_to_canvas(state->cursor), ' ');
  //   } else {
  //     // Print non-print characters to bottom left in status_win bar
  //     mvwaddch(status_win, 1, COLS - 3, state->ch_in);
  //   }
  // }
  // Move UI cursor to the right place
  wmove(canvas_win, cursor_y_to_canvas(state->cursor),
        cursor_x_to_canvas(state->cursor));

  return 0;
}
////////////////////////////
// SPECIFC MODE FUNCTIONS //
////////////////////////////

/* free_line_arrows_to_char
 *
 * Takes the current and previous arrow directions, and returns the
 * appropriate character, including diagonals.
 *
 * NOTE: Assumes that the input character is an arrow key
 *
 * Reference table:
 *       ^ v > <  (current)
 *
 *   ^   | | / \
 *   v   | | \ /
 *   >   \ / - -
 *   <   / \ - -
 * (last)
 */
int free_line_arrows_to_char(int last_arrow, int current_arrow) {
  char horizontal = '-';
  char vertical = '|';
  char diag_up = '/';
  char diag_down = '\\';

  if ((last_arrow == KEY_UP || current_arrow == KEY_DOWN) &&
      (last_arrow == KEY_DOWN || current_arrow == KEY_UP)) {
    // arrows are vertically parallel (top left quarter of truth table)
    return vertical;
  } else if ((last_arrow == KEY_LEFT || current_arrow == KEY_RIGHT) &&
             (last_arrow == KEY_RIGHT || current_arrow == KEY_LEFT)) {
    // arrows are horizontally parallel (bottom right quarter of truth table)
    return horizontal;
  } else if ((last_arrow == KEY_UP && current_arrow == KEY_RIGHT) ||
             (last_arrow == KEY_DOWN && current_arrow == KEY_LEFT) ||
             (last_arrow == KEY_LEFT && current_arrow == KEY_DOWN) ||
             (last_arrow == KEY_RIGHT && current_arrow == KEY_UP)) {
    return diag_up;
  } else {
    return diag_down;
  }
}
