#ifndef view_h
#define view_h

#include <ncurses.h>
#include <stdlib.h>
#include "canvas.h"

#define STATUS_HEIGHT 2  // not including borders
#define view_max_x (COLS - 2)
#define view_max_y (LINES - 3 - STATUS_HEIGHT)

typedef struct {
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

bool view_isin(View *view, int y, int x);
#endif
