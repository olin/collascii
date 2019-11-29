/* Frontend Modes
 *
 * This file contains the functions for different modes.
 *
 * IF YOU ADD A FUNCTION, ADD IT TO THE ENUM in "mode_id.h" AND the `modes`
 * array below, in the same order! All functions must have the same prototype of
 * `mode_function_t` to fit in modes.
 *
 * Mode functions are called with a `reason` enum and the current `state` state.
 * `reason` is the reason the function is being called:
 * - `START` is given after the mode has just been selected but before any
 *   keypresses have come in. This is a good time to update any relevant UI and
 *   initialize/reset any custom variables/states.
 * - `END` is given after the mode has been deselected, but before the new mode
 *   is initialized.
 * - `NEW_KEY` is given when a new keypress is ready for the mode to interpret.
 *   The keypress is stored in `state->ch_in` as an int of the ncurses character
 *   variety. https://invisible-island.net/ncurses/man/curs_getch.3x.html
 *
 * NOTE: if you update the `state->canvas` directly, changes won't be updated on
 * the ncurses `canvas_win` and you should call `redraw_canvas_win`.
 */

#include "fe_modes.h"

#include <errno.h>
#include <string.h>

#include <ncurses.h>

#include "frontend.h"
#include "mode_id.h"
#include "util.h"

// #define LOG_KEY_EVENTS  // `logd` new mouse and key events

editor_mode_t modes[] = {
    {"Switcher", "Switch to another mode", mode_picker},
    {"Insert", "Insert characters", mode_insert},
    {"Pan", "Pan around the canvas", mode_pan},
    {"Free-Line", "Draw a line with your arrow keys", mode_free_line},
    {"Brush", "Paint with arrow keys and mouse", mode_brush},
    {"", "", NULL},
    {"GOTO", "", mode_goto},
};

typedef struct {
  char pattern;
  enum { PAINT_ON, PAINT_OFF } state;
} mode_brush_config_t;

mode_brush_config_t mode_brush_config = {
    .pattern = 'B',
    .state = PAINT_OFF,
};

typedef struct {
  Cursor *last_dir_change;
} mode_insert_config_t;

mode_insert_config_t mode_insert_config = {NULL};

typedef struct {
  enum { ENTER_FIRST, ENTER_SECOND } state;
  char buffer[8];
  int xpos, ypos;
} mode_goto_config_t;

mode_goto_config_t mode_goto_config = {ENTER_FIRST, "", 0, 0};

///////////////////////
// GENERAL FUNCTIONS //
///////////////////////

void cmd_read_from_file(State *state) {
  FILE *f = fopen(state->filepath, "r");
  if (f == NULL) {
    perror("read_from_file");
    exit(1);
  }
  Canvas *old = state->view->canvas;
  state->view->canvas = canvas_readf(f);
  fclose(f);
  canvas_free(old);
  redraw_canvas_win();
}

void cmd_write_to_file(State *state) {
  FILE *f = fopen(state->filepath, "w");
  if (f == NULL) {
    perror("write_to_file");
    exit(1);
  }
  canvas_fprint_trim(f, state->view->canvas);
  fclose(f);
}

void cmd_trim_canvas(State *state) {
  Canvas *orig = state->view->canvas;
  state->view->canvas = canvas_trimc(orig, ' ', true, true, false, false);
  redraw_canvas_win();
}

/* Call a mode given its Mode_ID.
 *
 * This makes sure info_win is always updated.
 */
int call_mode(Mode_ID mode, reason_t reason, State *state) {
  int res = modes[mode].mode_function(reason, state);
  update_info_win_state(state);
  return res;
}

/* Wrapper of update_info_win for state
 */
inline void update_info_win_state(State *state) {
  update_info_win(state->current_mode, state->view->x + state->cursor->x,
                  state->view->y + state->cursor->y,
                  state->view->canvas->num_cols, state->view->canvas->num_rows);
}

/* Switch to a different mode.
 *
 * It sends END to the old mode and then START to the new mode
 */
void switch_mode(Mode_ID new_mode, State *state) {
  logd("Switching to %s\n", modes[new_mode].name);
  call_mode(state->current_mode, END, state);
  state->last_canvas_mode = state->current_mode;
  state->current_mode = new_mode;
  print_mode_win("");  // clear mode window;
  call_mode(new_mode, START, state);
  refresh_screen();
}

Mode_ID add_mod_canvas_mode(Mode_ID mode, int n) {
  int mode_first = MODE_PICKER + 1;  // beginning of selectable modes
  int mode_list_end = LAST;          // length of total mode list

  if (mode < mode_first || mode_list_end <= mode) {
    logd("mode \"%s\" is not a canvas mode", mode);
    return mode;
  }

  // loop properly between mode_first and last mode
  return ((mode - mode_first) + n) % (mode_list_end - mode_first) + mode_first;
}

Mode_ID next_canvas_mode(Mode_ID mode) { return add_mod_canvas_mode(mode, 1); }

Mode_ID previous_canvas_mode(Mode_ID mode) {
  return add_mod_canvas_mode(mode, -1);
}

/* Handler run in frontend main loop.
 *
 * Interprets keypresses, manages global keys, and passes data to modes.
 */
int master_handler(State *state, WINDOW *canvas_win, WINDOW *status_win) {
  // catching keypresses
  int c = wgetch(canvas_win);  // grab from window
#ifdef LOG_KEY_EVENTS
  logd("New key: '%c' (%d)\n", c, c);
#endif
  if (c == KEY_MOUSE) {
    // handle mouse events
    MEVENT event;
    if (getmouse(&event) == OK) {
#ifdef LOG_KEY_EVENTS
      logd("New mouse event: (%i, %i), %li\n", event.x, event.y, event.bstate);
#endif
      // TODO: look into mouse_trafo, wmouse_trafo
      // https://invisible-island.net/ncurses/man/curs_mouse.3x.html
      state->ch_in = c;
      state->mevent_in = &event;
      call_mode(state->current_mode, NEW_MOUSE, state);
    }
  } else if (c == KEY_TAB) {  // switching modes
    if (state->current_mode == MODE_PICKER &&
        state->last_canvas_mode != MODE_PICKER) {
      state->last_canvas_mode = next_canvas_mode(state->last_canvas_mode);

      // update mode_picker() to redraw the highlight
      state->ch_in = c;
      call_mode(state->current_mode, NEW_KEY, state);
    } else {
      switch_mode(MODE_PICKER, state);
    }
    return 0;
  } else if (c == KEY_SHIFT_TAB) {
    if (state->current_mode == MODE_PICKER &&
        state->last_canvas_mode != MODE_PICKER) {
      state->last_canvas_mode = previous_canvas_mode(state->last_canvas_mode);

      // update mode_picker() to redraw the highlight
      state->ch_in = c;
      call_mode(state->current_mode, NEW_KEY, state);
    } else {
      switch_mode(MODE_PICKER, state);
    }
  } else if (c == KEY_NPAGE || c == KEY_PPAGE || c == KEY_SLEFT ||
             c == KEY_SRIGHT) {
    // shift view down/up/left/right
    // renaming for ease of use
    const int h = getmaxy(canvas_win) - 2;  // height of visible canvas
    const int w = getmaxx(canvas_win) - 2;  // width
    const int vy = state->view->y;
    const int vx = state->view->x;
    const int ch = state->view->canvas->num_rows;
    const int cw = state->view->canvas->num_cols;
    int new_vy = vy;
    int new_vx = vx;
    // calc shift based on key
    switch (c) {
      case KEY_NPAGE:  // down
        new_vy = min(vy + h, ch - h);
        break;
      case KEY_PPAGE:  // up
        new_vy = max(0, vy - h);
        break;
      case KEY_SRIGHT:
        new_vx = min(vx + w, cw - w);
        break;
      case KEY_SLEFT:
        new_vx = max(0, vx - w);
    }
    // shift view
    state->view->y = new_vy;
    state->view->x = new_vx;
    redraw_canvas_win();
    update_info_win_state(state);
  } else if (c == KEY_CTRL('r')) {
    cmd_read_from_file(state);
    print_msg_win("Read from file '%s'\n", state->filepath);
  } else if (c == KEY_CTRL('s')) {
    cmd_write_to_file(state);
    print_msg_win("Saved to file '%s'\n", state->filepath);
  } else if (c == KEY_CTRL('t')) {
    cmd_trim_canvas(state);
  } else if (c == KEY_CTRL('g')) {
    switch_mode(MODE_GOTO, state);
  } else {
    // pass character on to mode
    state->ch_in = c;
    call_mode(state->current_mode, NEW_KEY, state);
  }

  // Move UI cursor to the right place
  wmove(canvas_win, cursor_y_to_canvas(state->cursor),
        cursor_x_to_canvas(state->cursor));
  return 0;
}

/////////////////////////////
// SPECIFIC MODE FUNCTIONS //
/////////////////////////////

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
 * Mode to switch between other modes using number keys.
 */
int mode_picker(reason_t reason, State *state) {
  if (reason == END) {
    return 0;
  }

  // get bounds of switchable modes in enum array (DON'T include mode_picker)
  int mode_first = MODE_PICKER + 1;  // beginning of selectable modes
  int mode_list_end = LAST;          // length of total mode list

  // BUILD MODE INFO MESSAGE
  char msg[128] = "";
  int num_left = sizeof(msg) / sizeof(char);

  char buffer[16];

  // these variables are used to reverse-video the current canvas mode
  int selected_x = 0;
  int selected_num_char = -1;
  bool found_selected = FALSE;
  for (int i = mode_first; i < mode_list_end; i++) {
    int num_to_write = snprintf(buffer, sizeof(buffer) / sizeof(char),
                                " [%i] %s ", i - mode_first + 1, modes[i].name);

    if (num_left - num_to_write < 0) {
      break;
    }
    strncat(msg, buffer, num_to_write);
    num_left -= num_to_write;

    // Find the current canvas mode
    if (state->last_canvas_mode == i) {
      found_selected = TRUE;
      selected_num_char = num_to_write;
    }

    // Keep track of where the selected mode text starts in the window
    if (!found_selected) {
      selected_x += num_to_write;
    }
  }

  print_mode_win(msg);

  // Reverse-video the current canvas mode
  if (selected_num_char != -1) {
    highlight_mode_text(selected_x, selected_num_char);
  }

  // INTERPRET KEYS
  if (reason == NEW_KEY) {
    // only accept characters within the bounds of the list
    if (state->ch_in >= '1' &&
        state->ch_in < '1' + mode_list_end - mode_first) {
      Mode_ID new_mode = mode_first + state->ch_in - '1';
      switch_mode(new_mode, state);
      return 0;
    } else if (state->ch_in == KEY_ENTER) {
      switch_mode(state->last_canvas_mode, state);
      return 0;
    }
  }
  return 0;
}

/* mode_insert
 *
 * Move with arrows and insert character with keyboard.
 */
int mode_insert(reason_t reason, State *state) {
  if (reason != NEW_KEY) {
    return 0;
  }

  mode_insert_config_t *mode_cfg = &mode_insert_config;
  // init config cursor
  if (mode_cfg->last_dir_change == NULL) {
    mode_cfg->last_dir_change = cursor_copy(state->cursor);
  }

  // watch for view shifts
  const int vx = state->view->x;
  const int vy = state->view->y;
  // insert mode behavior
  if ((state->ch_in == KEY_LEFT) || (state->ch_in == KEY_RIGHT) ||
      (state->ch_in == KEY_UP) || (state->ch_in == KEY_DOWN)) {
    cursor_key_to_move(state->ch_in, state->cursor, state->view);
    state->last_arrow_direction = state->ch_in;

    // update direction change cursor
    Cursor *old = mode_cfg->last_dir_change;
    mode_cfg->last_dir_change = cursor_copy(state->cursor);
    cursor_free(old);
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
    } else if (state->ch_in == KEY_ENTER) {
      // return to beginning of "line" on ENTER
      // TODO: check if out of view and recenter
      if (state->last_arrow_direction == KEY_RIGHT ||
          state->last_arrow_direction == KEY_LEFT) {
        // return down if left/right
        state->cursor->x = mode_cfg->last_dir_change->x;
        cursor_move_down(state->cursor, state->view);
        mode_cfg->last_dir_change->y = state->cursor->y;
      } else if (state->last_arrow_direction == KEY_UP ||
                 state->last_arrow_direction == KEY_DOWN) {
        // return right if up/down
        state->cursor->y = mode_cfg->last_dir_change->y;
        cursor_move_right(state->cursor, state->view);
        mode_cfg->last_dir_change->x = state->cursor->x;
      }
    }
  }
  // update last_dir_change with view diffs
  const int dx = vx - state->view->x;
  const int dy = vy - state->view->y;
  if (dx != 0) {
    mode_cfg->last_dir_change->x += dx;
  }
  if (dy != 0) {
    mode_cfg->last_dir_change->y += dy;
  }
  return 0;
}

/* mode_pan
 *
 * Pans the View with arrow keys
 */
int mode_pan(reason_t reason, State *state) {
  if (reason != NEW_KEY) {
    return 0;
  }

  if ((state->ch_in == KEY_LEFT) || (state->ch_in == KEY_RIGHT) ||
      (state->ch_in == KEY_UP) || (state->ch_in == KEY_DOWN)) {
    view_pan_ch(state->ch_in, state->view);
  }

  redraw_canvas_win();
  return 0;
}

/* mode_free_line
 *
 * Move with arrows and insert character with keyboard.
 */
int mode_free_line(reason_t reason, State *state) {
  if (reason != NEW_KEY) {
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
int mode_brush(reason_t reason, State *state) {
  // brush mode behavior

  mode_brush_config_t *mode_cfg = &mode_brush_config;

  if (reason == START) {
    // make sure brush always starts off
    mode_cfg->state = PAINT_OFF;
  }

  if (reason == NEW_MOUSE) {
    if (state->mevent_in->bstate & BUTTON1_PRESSED) {
      mode_cfg->state = PAINT_ON;
    } else if (state->mevent_in->bstate & BUTTON1_RELEASED) {
      mode_cfg->state = PAINT_OFF;
    }
    // only update cursor position on mouse move if we're painting
    if (mode_cfg->state == PAINT_ON) {
      state->cursor->x = state->mevent_in->x - 1;
      state->cursor->y = state->mevent_in->y - 1;
    }
  } else if (reason == NEW_KEY) {
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
    }
  }

  // if painting, change character
  if (mode_cfg->state == PAINT_ON) {
    front_setcharcursor(mode_cfg->pattern);
  }

  // display brush info
  print_mode_win("state: %s\tbrush: '%c' (Press ENTER to toggle)",
                 ((mode_cfg->state == PAINT_OFF) ? "OFF" : "ON"),
                 mode_cfg->pattern);

  return 0;
}

int update_pos_value(mode_goto_config_t *mode_cfg) {
  int *pos;
  switch (mode_cfg->state) {
    case ENTER_FIRST:
      pos = &(mode_cfg->xpos);
      break;
    case ENTER_SECOND:
      pos = &(mode_cfg->ypos);
  }
  char *buff = mode_cfg->buffer;
  // parse buffer value
  errno = 0;
  int val = (int)strtol(buff, NULL, 10);
  if (errno != 0) {
    perror("update_pos_value strtol");
    print_mode_win("Invalid number");
    return 1;
  }
  // move relative if sign is present
  switch (buff[0]) {
    case '-':
    case '+':
      *pos = *pos + val;
      break;
    default:
      *pos = val;
      break;
  }
  return 0;
}

/* mode_goto
 *
 * Move the cursor to a position entered on the keyboard.
 */
int mode_goto(reason_t reason, State *state) {
  mode_goto_config_t *mode_cfg = &mode_goto_config;
  char *buff = mode_cfg->buffer;
  if (reason == START) {
    // reset mode state
    mode_cfg->state = ENTER_FIRST;
    mode_cfg->xpos = state->view->x + state->cursor->x;
    mode_cfg->ypos = state->view->y + state->cursor->y;
    memset(buff, 0, 8);
  }
  if (reason != NEW_KEY) {
    return 0;
  }

  switch (state->ch_in) {
    case ',':
      // parse, switch to second value
      update_pos_value(mode_cfg);
      memset(buff, 0, 8);  // reset buffer
      mode_cfg->state = ENTER_SECOND;
      break;
    case KEY_ENTER:
      // parse, set cursor location and return to last mode
      update_pos_value(mode_cfg);
      memset(buff, 0, 8);  // reset buffer
      state->cursor->x = mode_cfg->xpos - state->view->x;
      state->cursor->y = mode_cfg->ypos - state->view->y;
      // TODO: return somehow
      break;
    case KEY_BACKSPACE: {
      // remove last char in buffer
      int l = strlen(buff);
      if (l > 0) {
        buff[l] = '\0';
      }
    }
    default: {
      // check if valid and add to buffer
      // TODO: check for validity
      int l = strlen(buff);
      if (l < 8 - 1) {
        buff[l] = state->ch_in;
      }
      break;
    }
  }

  print_mode_win("Go to pos: (%i, %i)", mode_cfg->xpos, mode_cfg->ypos);
  return 0;
}
