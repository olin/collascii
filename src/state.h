#ifndef state_h
#define state_h

#include "cursor.h"
#include "mode_id.h"
#include "view.h"

typedef struct {
  int uid;
  int y;
  int x;
} collab_t;

typedef struct {
  int num;
  int len;
  collab_t **list;
} collab_list_t;

/* State keeps track of changing variables for mode functions.
 * If you add something, don't forget to also add an init before the main
 * loop.
 *
 * TODO: replace last_arrow with linked list of previous inputs?
 * Make sure its length is capped.
 */
typedef struct {
  int ch_in;                 // characters read by ncurses
  MEVENT *mevent_in;         // mouse events read by ncurses
  Cursor *cursor;            // cursor location on screen, relative to window
  View *view;                // view of the canvas (also holds the canvas)
  Mode_ID current_mode;      // the current mode of the interface
  Mode_ID last_canvas_mode;  // the last mode of the interface
  int last_arrow_direction;
  Cursor *last_cursor;
  char *filepath;  // path of savefile

  // network-related
  collab_list_t *collab_list;
  char *name;
} State;

#endif
