#include "fe_modes.h"

#include "frontend.h"
#include "mode_id.h"
#include "util.h"

#include <string.h>

/* Frontend Modes
 *
 * This file contains the functions for different modes.
 *
 * IF YOU ADD A FUNCTION, ADD IT TO THE ENUM in "mode_id.h" AND the `modes`
 * array below! All functions must have the same signature of `mode_function_t`
 * to fit in modes.
 *
 */

typedef struct {
  char pattern;
  enum { PAINT_ON, PAINT_OFF } state;
} mode_brush_config_t;

mode_brush_config_t mode_brush_config = {
    .pattern = 'B',
    .state = PAINT_OFF,
};

editor_mode_t modes[] = {
    {"Mode Selector", "", mode_picker},
    {"Insert", "", mode_insert},
    {"Pan", "", mode_pan},
    {"Free-Line", "", mode_free_line},
    {"Brush", "", mode_brush},
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

////////////////////
// MODE FUNCTIONS //
////////////////////

/* mode_status
 *
 * Default mode. Used to choose other modes.
 */
int mode_picker(State *state, WINDOW *canvas_win, WINDOW *status_win) {
  int mode_start = MODE_PICKER + 1;
  int num_modes = LAST;

  // BUILD MESSAGE
  char msg[128] = "";
  int num_left = sizeof(msg) / sizeof(char);

  char buffer[16];
  for (int i = mode_start; i < num_modes; i++) {
    int num_to_write = snprintf(buffer, sizeof(buffer) / sizeof(char),
                                "%i: %s|", i - mode_start + 1, modes[i].name);
    if (num_left - num_to_write < 0) {
      break;
    }
    strncat(msg, buffer, num_to_write);
    num_left -= num_to_write;
  }

  // print_status("foo");
  // wrefresh(status_win);
  print_status(msg);
  wrefresh(status_win);

  // INTERPRET KEYS
  if (state->ch_in == KEY_TAB) {
    state->current_mode = state->last_canvas_mode;
    print_status("");
    return 0;
  } else if (state->ch_in >= '1' &&
             state->ch_in < '1' + num_modes - mode_start) {
    state->last_canvas_mode = MODE_PICKER;
    state->current_mode = mode_start + state->ch_in - '1';
    print_status("");
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

  // free line behavior
  if ((state->ch_in == KEY_LEFT) || (state->ch_in == KEY_RIGHT) ||
      (state->ch_in == KEY_UP) || (state->ch_in == KEY_DOWN)) {
    int current_arrow = state->ch_in;
    int last_arrow = state->last_arrow_direction;

    front_setcharcursor(free_line_arrows_to_char(last_arrow, current_arrow));

    state->last_cursor->x = state->cursor->x;
    state->last_cursor->y = state->cursor->y;

    cursor_key_to_move(current_arrow, state->cursor, state->view);

    state->last_arrow_direction = state->ch_in;
  } else if (state->ch_in == KEY_BACKSPACE) {
    cursor_key_to_move(cursor_opposite_dir(state->last_arrow_direction),
                       state->cursor, state->view);
    front_setcharcursor(' ');
  }

  wmove(canvas_win, cursor_y_to_canvas(state->cursor),
        cursor_x_to_canvas(state->cursor));

  return 0;
}

/* mode_brush
 *
 * Continuous painting of characters.
 *
 * Toggle on/off with ENTER, change characters by pressing them.
 *
 * TODO: allow multi-character patterns
 * TODO: change "radius" of stroke
 */
int mode_brush(State *state, WINDOW *canvas_win, WINDOW *status_win) {
  // handle mode changing
  if (state->ch_in == KEY_TAB) {
    // Clean up code
    state->last_canvas_mode = MODE_BRUSH;

    state->current_mode = MODE_PICKER;
    return 0;
  }

  // brush mode behavior

  mode_brush_config_t *mode_cfg = &mode_brush_config;

  if ((state->ch_in == KEY_LEFT) || (state->ch_in == KEY_RIGHT) ||
      (state->ch_in == KEY_UP) || (state->ch_in == KEY_DOWN)) {
    // arrow keys - move cursor
    cursor_key_to_move(state->ch_in, state->cursor, state->view);
  } else if (' ' <= state->ch_in && state->ch_in <= '~') {
    // printable characters - change brush
    mode_cfg->pattern = state->ch_in;
  } else if (KEY_ENTER == state->ch_in) {
    // ENTER - toggle on/off
    if (mode_cfg->state == PAINT_ON) {
      mode_cfg->state = PAINT_OFF;
    } else if (mode_cfg->state == PAINT_OFF) {
      mode_cfg->state = PAINT_ON;
    }
  } else if (KEY_MOUSE == state->ch_in) {
    // handle mouse events
    MEVENT event;
    if (getmouse(&event) == OK) {
      logd("New mouse event: (%i, %i), %li\n", event.x, event.y, event.bstate);
      if (event.bstate & BUTTON1_PRESSED) {
        mode_cfg->state = PAINT_ON;
      } else if (event.bstate & BUTTON1_RELEASED) {
        mode_cfg->state = PAINT_OFF;
      }
      // move cursor to mouse position when PAINT_ON
      if (mode_cfg->state == PAINT_ON &&
          wenclose(canvas_win, event.y, event.x)) {
          state->cursor->x = event.x - 1;
          state->cursor->y = event.y - 1;
      }
    }
  } else {
    // Print non-print characters to bottom left in status_win bar
    mvwaddch(status_win, 1, COLS - 3, state->ch_in);
    logd("Keycode: %i\n", state->ch_in);
  }

  // if painting, change character
  if (mode_cfg->state == PAINT_ON) {
    front_setcharcursor(mode_cfg->pattern);
  }

  // display brush info
  print_status("state: %s\tbrush: '%c' (Press ENTER to toggle)",
               ((mode_cfg->state == PAINT_OFF) ? "OFF" : "ON"),
               mode_cfg->pattern);

  return 0;
}
