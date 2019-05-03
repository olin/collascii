#include "cursor.h"
#include <ncurses.h>
#include <stdlib.h>
#include "frontend.h"

/* The Cursor struct helps with controls.
 * It also maps the drawing area to the canvas nicely.
 */

Cursor *cursor_new() {
  Cursor *cursor = malloc(sizeof(Cursor));
  cursor->x = 0;
  cursor->y = 0;
  return cursor;
}

void cursor_move_up(Cursor *cursor, View *view) {
  if (cursor->y == 0) {
    view_move_up(view);
    return;
  }
  cursor->y--;
}

void cursor_move_down(Cursor *cursor, View *view) {
  if (cursor->y == view->canvas->num_rows - view->y - 2) {
    return;
  }
  if (cursor->y == view_max_y) {
    view_move_down(view);
    return;
  }
  cursor->y++;
}

void cursor_move_left(Cursor *cursor, View *view) {
  if (cursor->x == 0) {
    view_move_left(view);
    return;
  }
  cursor->x--;
}

void cursor_move_right(Cursor *cursor, View *view) {
  if (cursor->x == view->canvas->num_cols - view->x - 2) {
    return;
  }
  if (cursor->x == view_max_x) {
    view_move_right(view);
    return;
  }
  cursor->x++;
}

int cursor_x_to_canvas(Cursor *cursor) { return cursor->x + 1; }

int cursor_y_to_canvas(Cursor *cursor) { return cursor->y + 1; }

void cursor_key_to_move(int arrow, Cursor *cursor, View *view) {
  switch (arrow) {
    case KEY_LEFT:
      cursor_move_left(cursor, view);
      break;
    case KEY_RIGHT:
      cursor_move_right(cursor, view);
      break;
    case KEY_UP:
      cursor_move_up(cursor, view);
      break;
    case KEY_DOWN:
      cursor_move_down(cursor, view);
      break;
  }
}

int cursor_opposite_dir(int arrow) {
  switch (arrow) {
    case KEY_LEFT:
      return KEY_RIGHT;
    case KEY_RIGHT:
      return KEY_LEFT;
    case KEY_UP:
      return KEY_DOWN;
    case KEY_DOWN:
      return KEY_UP;
  }
  return -1;
}

void cursor_free(Cursor *cursor) { free(cursor); }
