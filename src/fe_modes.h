#ifndef fe_modes_h
#define fe_modes_h

#include <ncurses.h>
#include "state.h"

int mode_master(State *state, WINDOW *canvas_win, WINDOW *status_win);

typedef enum { START, NEW_KEY, NEW_MOUSE, END } reason_t;

// Prototype for mode functions. Note that this is NOT a function pointer type
// (use `mode_function_t*` for that). https://stackoverflow.com/a/5195682
typedef int mode_function_t(reason_t, State *);

mode_function_t mode_picker;
mode_function_t mode_insert;
mode_function_t mode_pan;
mode_function_t mode_line;
mode_function_t mode_free_line;
mode_function_t mode_brush;

typedef struct {
  char *name;
  char *description;
  mode_function_t *mode_function;
} editor_mode_t;

extern editor_mode_t modes[];

int master_handler(State *state, WINDOW *canvas_win, WINDOW *status_win);
void switch_mode(Mode_ID new_mode, State *state);
int call_mode(Mode_ID mode, reason_t reason, State *state);
void update_info_win_state(State *state);

void read_from_file(State *state);
void write_to_file(State *state);

#endif
