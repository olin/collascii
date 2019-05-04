#ifndef view_h
#define view_h

#include <ncurses.h>
#include <stdlib.h>
#include "canvas.h"

#define STATUS_HEIGHT 2  // not including borders

// define zero-indexed positions from top left corner of canvas view
//                       - 3: -2 for borders and -1 for zero-index
#define view_max_x (COLS - 3)
//                        - 4: -3 for borders -1 for zero-index
#define view_max_y (LINES - 4 - STATUS_HEIGHT)

typedef struct {
  // top-left of view, in zero-indexed canvas coordinates
  int x, y;
  Canvas *canvas;
} View;

View *view_new(Canvas *canvas);
View *view_new_startpos(Canvas *canvas, int x, int y);

void view_move_up(View *view);
void view_move_down(View *view);
void view_move_left(View *view);
void view_move_right(View *view);

void view_pan_ch(int ch, View *view);

#endif
