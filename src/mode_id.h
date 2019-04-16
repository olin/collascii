#ifndef mode_id_h
#define mode_id_h

/* IMPORTANT: Ensure that the order of the enum is the same as mode_functions
 *
 * This enum is used to index into the mode_functions array.
 */
typedef enum {
  MODE_PICKER,
  MODE_INSERT,
  MODE_PAN,

  // ^ add your mode above
  LAST,  // used to get number of elements
} Mode_ID;

#endif
