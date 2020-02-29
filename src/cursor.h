#ifndef cursor_h
#define cursor_h

/* The Cursor struct helps with controls.
 * It also maps the drawing area to the canvas nicely.
 */

#include "view.h"

#include <ncurses.h>
#include <stdlib.h>

typedef struct CURSOR {
  int x;
  int y;
} Cursor;

Cursor *cursor_new();
Cursor *cursor_newyx(int y, int x);
Cursor *cursor_copy(Cursor *original);
void cursor_free(Cursor *cursor);

bool cursor_isin_view(Cursor *cursor, View *view);

void cursor_move_up(Cursor *cursor, View *view);
void cursor_move_down(Cursor *cursor, View *view);
void cursor_move_left(Cursor *cursor, View *view);
void cursor_move_right(Cursor *cursor, View *view);

int cursor_x_to_canvas(Cursor *cursor);
int cursor_y_to_canvas(Cursor *cursor);
void cursor_key_to_move(int arrow, Cursor *cursor, View *view);
int cursor_opposite_dir(int arrow);

#endif
