#ifndef mode_id_h
#define mode_id_h

/* IMPORTANT: Ensure that the order of the enum is the same as modes in
 * "fe_modes.c"
 *
 * This enum is used to index into the modes array.
 */
typedef enum {
  MODE_PICKER,
  MODE_INSERT,
  MODE_PAN,
  MODE_FREE_LINE,
  MODE_BRUSH,

  // ^ add your mode above
  LAST,  // used to get number of elements
} Mode_ID;

#endif
