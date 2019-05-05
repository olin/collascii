#ifndef fe_modes_h
#define fe_modes_h

#include <ncurses.h>
#include "state.h"

int mode_picker(State *state, WINDOW *canvas_win, WINDOW *status_win);
int mode_insert(State *state, WINDOW *canvas_win, WINDOW *status_win);
int mode_pan(State *state, WINDOW *canvas_win, WINDOW *status_win);
int mode_free_line(State *state, WINDOW *canvas_win, WINDOW *status_win);

extern int (*mode_functions[])(State *, WINDOW *, WINDOW *);

#endif
