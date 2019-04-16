#ifndef cursor_h
#define cursor_h

/* The Cursor struct helps with controls.
 * It also maps the drawing area to the canvas nicely.
 */

typedef struct CURSOR {
  int x;
  int y;
} Cursor;

Cursor* cursor_new();
void cursor_free(Cursor* cursor);

void cursor_move_up(Cursor* cursor);
void cursor_move_down(Cursor* cursor);
void cursor_move_left(Cursor* cursor);
void cursor_move_right(Cursor* cursor);

int cursor_x_to_canvas(Cursor* cursor);
int cursor_y_to_canvas(Cursor* cursor);
void cursor_key_to_move(int arrow, Cursor* cursor);
int cursor_opposite_dir(int arrow);

#endif
