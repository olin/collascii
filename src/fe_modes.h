#ifndef fe_modes_h
#define fe_modes_h

#include <ncurses.h>
#include "state.h"

int mode_picker(State *state, WINDOW *canvas_win, WINDOW *status_win);
int mode_insert(State *state, WINDOW *canvas_win, WINDOW *status_win);
int mode_pan(State *state, WINDOW *canvas_win, WINDOW *status_win);
int mode_free_line(State *state, WINDOW *canvas_win, WINDOW *status_win);
int mode_brush(State *state, WINDOW *canvas_win, WINDOW *status_win);

typedef int (*mode_function_t)(State *, WINDOW *, WINDOW *);

typedef struct {
  char *name;
  char *description;
  mode_function_t mode_function;
} editor_mode_t;

extern editor_mode_t modes[];

#endif
