#include "cursor.h"
#include <ncurses.h>
#include <stdlib.h>
#include "frontend.h"

/* The Cursor struct helps with controls.
 * It also maps the drawing area to the canvas nicely.
 */

/* Make a new cursor.
 *
 * Initialized to (0, 0).
 */
Cursor *cursor_new() {
  return cursor_newyx(0, 0);
}

/* Make a new cursor at a given position.
 */
Cursor *cursor_newyx(int y, int x) {
  Cursor *cursor = malloc(sizeof(Cursor));
  cursor->x = x;
  cursor->y = y;
  return cursor;
}

/* Make a copy of an existing cursor.
 *
 */
Cursor *cursor_copy(Cursor *original) {
  return cursor_newyx(original->y, original->x);
}

void cursor_move_up(Cursor *cursor, View *view) {
  if (cursor->y == 0) {
    view_move_up(view);
    return;
  }
  cursor->y--;
}

void cursor_move_down(Cursor *cursor, View *view) {
  if (cursor->y + 1 >= view->canvas->num_rows - view->y) {
    cursor->y = view->canvas->num_rows - view->y - 1;
    return;
  }
  if (cursor->y + 1 >= view_max_y) {
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
  if (cursor->x >= view->canvas->num_cols - view->x - 1) {
    cursor->x = view->canvas->num_cols - view->x - 1;
    return;
  }
  if (cursor->x + 1 >= view_max_x) {
    view_move_right(view);
    return;
  }
  cursor->x++;
}

int cursor_x_to_canvas(Cursor *cursor) {
  return cursor->x + 1;
}

int cursor_y_to_canvas(Cursor *cursor) {
  return cursor->y + 1;
}

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

void cursor_free(Cursor *cursor) {
  free(cursor);
}
