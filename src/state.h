#ifndef state_h
#define state_h

#include "cursor.h"
#include "view.h"
#include "mode_id.h"

/* State keeps track of changing variables for mode functions.
 * If you add something, don't forget to also add an init before the main loop.
 *
 * TODO: replace last_arrow with linked list of previous inputs?
 * Make sure its length is capped.
 */
typedef struct
{
  int ch_in;
  Cursor *cursor;
  Mode_ID current_mode;

  Mode_ID last_canvas_mode;
  int last_arrow_direction;
  View *view;
} State;

#endif
