#include <stdlib.h>
#include <ncurses.h>
#include "cursor.h"

/* The Cursor struct helps with controls.
 * It also maps the drawing area to the canvas nicely.
 */

Cursor *cursor_new() {
    Cursor *cursor = malloc(sizeof(Cursor));
    cursor->x = 0;
    cursor->y = 0; 
    return cursor;
}

void cursor_move_up(Cursor* cursor) {
    if (cursor->y == 0) {
        return;
    }
    cursor->y--;
}

void cursor_move_down(Cursor* cursor) {
    if (cursor->y == (LINES - 2)) { // Take box lines into account
        return;
    }
    cursor->y++;
}

void cursor_move_left(Cursor* cursor) {
    if (cursor->x == 0) {
        return;
    }
    cursor->x--;
}

void cursor_move_right(Cursor* cursor) {
    if (cursor->x == (COLS - 2)) { // Take box lines into account
        return;
    }
    cursor->x++;
}

int cursor_x_to_canvas(Cursor* cursor) {
    return cursor->x + 1;
}

int cursor_y_to_canvas(Cursor* cursor) {
    return cursor->y + 1;
}

void cursor_key_to_move(int arrow, Cursor *cursor) {
    switch(arrow) {	
        case KEY_LEFT:
            cursor_move_left(cursor);
            break;
        case KEY_RIGHT:
            cursor_move_right(cursor);
            break;
        case KEY_UP:
            cursor_move_up(cursor);
            break;
        case KEY_DOWN:
            cursor_move_down(cursor);
            break;
    }
}

void cursor_free(Cursor* cursor) {
    free(cursor);
}
