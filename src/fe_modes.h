#ifndef fe_modes_h
#define fe_modes_h

#include <ncurses.h>
#include "frontend.h"

int mode_status(State *state, WINDOW *canvas_win, WINDOW *status_win);
int mode_arrow_input(State *state, WINDOW *canvas_win, WINDOW *status_win);

int (*mode_functions[]) (State*, WINDOW*, WINDOW*) = {
    mode_status,
    mode_arrow_input,
};

#endif
