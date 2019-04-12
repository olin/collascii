#ifndef fe_modes_h
#define fe_modes_h

#include <ncurses.h>
#include "frontend.h"

/* IMPORTANT: Ensure that the order of the enum is the same as mode_functions
 *
 * This enum is used to index into the mode_functions array.
 */ 
typedef enum mode_id {
    PICKER,
    INSERT,

    // ^ add your mode above (LAST is used to get number of elements)
    LAST,
} Mode_ID;

int mode_picker(State *state, WINDOW *canvas_win, WINDOW *status_win);
int mode_arrow_input(State *state, WINDOW *canvas_win, WINDOW *status_win);

extern int (*mode_functions[]) (State*, WINDOW*, WINDOW*);

#endif
