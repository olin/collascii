/* API for Canvas objects
 *
 * TODO: add save/read from file options
 * TODO: assert get/set positions are within canvas sizes?
 */
#include "canvas.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "canvas.h"
#include "util.h"

/* Fill a canvas with char fill
 *
 */
void canvas_fill(Canvas *canvas, char fill) {
  for (int i = 0; i < canvas->num_rows; i++) {
    memset(canvas->rows[i], ' ', canvas->num_cols);
  }
}

/* Create a canvas object
 *
 * Returned pointer should be freed with free_canvas
 */
Canvas *canvas_new(int rows, int cols) {
  Canvas *canvas = malloc(sizeof(Canvas));
  canvas->num_cols = cols;
  canvas->num_rows = rows;
  canvas->rows = malloc(rows * sizeof(char *));
  for (int i = 0; i < rows; i++) {
    canvas->rows[i] = malloc(cols * sizeof(char));
  }
  canvas_fill(canvas, ' ');
  return canvas;
}

Canvas *canvas_new_blank(int rows, int cols) {
  Canvas *canvas = canvas_new(rows, cols);
  for (int i = 0; i < (rows * cols); i++) {
    canvas_schari(canvas, i, ' ');
  }
  return canvas;
}

/* Create and return a deep copy of a canvas
 *
 * Returned pointer should be freed with free_canvas
 */
Canvas *canvas_cpy(Canvas *orig) {
  // allocate new canvas
  Canvas *copy = canvas_new(orig->num_rows, orig->num_cols);
  // copy rows over from orig
  for (int i = 0; i < orig->num_rows; i++) {
    memcpy(copy->rows[i], orig->rows[i], sizeof(char) * orig->num_cols);
  }

  return copy;
}

/* Make a copy of a subset of a canvas.
 *
 * Copies the rectangle formed by points (x1, y1) and (x2, y2) inclusively. The
 * relative order and location of the two points does not matter.
 */
Canvas *canvas_cpy_p1p2(Canvas *orig, int y1, int x1, int y2, int x2) {
  // assert that points are within canvas
  assert(x1 >= 0 && x1 < orig->num_cols);
  assert(x2 >= 0 && x2 < orig->num_cols);
  assert(y1 >= 0 && y1 < orig->num_rows);
  assert(y2 >= 0 && y2 < orig->num_rows);

  // find the top left and bottom right points of the rectangle
  const int tlx = min(x1, x2);
  const int tly = min(y1, y2);

  // const int brx = max(x1, x2);  // unused
  const int bry = max(y1, y2);

  // make empty canvas
  const int width = abs(x2 - x1) + 1;
  const int height = abs(y2 - y1) + 1;
  Canvas *copy = canvas_new(height, width);
  // copy relevant data from orig, row-sections at a time
  for (int y = tly; y <= bry; y++) {
    memcpy(copy->rows[y], &(orig->rows[y][tlx]), sizeof(char) * width);
  }

  return copy;
}

/* Free a canvas object
 *
 */
void canvas_free(Canvas *canvas) {
  // free each row first
  for (int i = 0; i < canvas->num_rows; i++) {
    free(canvas->rows[i]);
  }
  // free array of rows
  free(canvas->rows);
  // free struct itself
  free(canvas);
}

/* Test if location y is inside canvas.
 */
int canvas_isin_y(Canvas *canvas, int y) {
  return (0 <= y && y < canvas->num_rows);
}

/* Test if location x is inside canvas.
 */
int canvas_isin_x(Canvas *canvas, int x) {
  return (0 <= x && x < canvas->num_cols);
}

/* Test if location (x, y) is inside canvas.
 */
int canvas_isin_yx(Canvas *canvas, int y, int x) {
  return (canvas_isin_y(canvas, y) && canvas_isin_x(canvas, x));
}

/* Test if index i is inside canvas.
 */
int canvas_isin_i(Canvas *canvas, int i) {
  return (i >= 0 && i < canvas->num_rows * canvas->num_cols);
}

/* Load canvas source into dest at point (x, y).
 *
 * Any parts of source that fall outside of dest will not be copied.
 *
 * Asserts that (y, x) is inside dest.
 *
 * Returns 1 if source canvas was truncated, 0 otherwise.
 */
int canvas_ldcanvasyx(Canvas *dest, Canvas *source, int y, int x) {
  assert(canvas_isin_yx(dest, y, x));
  const int max_height = dest->num_rows - y;
  const int max_width = dest->num_cols - x;

  // find bounds of destination
  const int copy_height = min(max_height, source->num_rows);
  const int copy_width = min(max_width, source->num_cols);

  // copy range over
  for (int i = 0; i < copy_height; i++) {
    memcpy(&(dest->rows[y + i][x]), source->rows[i], sizeof(char) * copy_width);
  }

  // figure out if source canvas was truncated
  if (max_height < source->num_rows || max_width < source->num_cols) {
    return 1;
  } else {
    return 0;
  }
}

/* Load canvas source into dest at point (x, y), ignoring char transparent.
 *
 * Any parts of source that fall outside of dest will not be copied.
 *
 * Asserts that (y, x) is inside dest.
 *
 * Returns 1 if source canvas was truncated, 0 otherwise.
 */
int canvas_ldcanvasyxc(Canvas *dest, Canvas *source, int y, int x,
                       char transparent) {
  assert(canvas_isin_yx(dest, y, x));
  const int max_height = dest->num_rows - y;
  const int max_width = dest->num_cols - x;

  // find bounds of destination
  const int copy_height = min(max_height, source->num_rows);
  const int copy_width = min(max_width, source->num_cols);

  // copy range over
  char c;
  for (int i = 0; i < copy_height; i++) {
    for (int j = 0; j < copy_width; j++) {
      c = source->rows[i][j];
      if (c != transparent) {
        // copy only if not transparent
        dest->rows[y + i][x + j] = c;
      }
    }
  }

  // figure out if source canvas was truncated
  if (max_height < source->num_rows || max_width < source->num_cols) {
    return 1;
  } else {
    return 0;
  }
}

/* Change the size of a canvas.
 *
 * Creates a new canvas, copies the content over, and frees the old canvas. Any
 * data falling outside the bounds of the new canvas is dropped.
 *
 * Requires a pointer to a canvas pointer.
 *
 * Returns 1 if the canvas was truncated, 0 otherwise.
 */
int canvas_resize(Canvas **canvas_pointer, int newrows, int newcols) {
  Canvas *orig = *canvas_pointer;
  Canvas *new = canvas_new(newrows, newcols);

  // copy over
  int res = canvas_ldcanvasyx(new, orig, 0, 0);

  // update and free
  *canvas_pointer = new;
  canvas_free(orig);
  return res;
}

/* Change the size of a canvas.
 *
 * Creates a new canvas, copies the content over, and frees the old canvas. Any
 * data falling outside the bounds of the new canvas is dropped.
 *
 * Requires a pointer to a canvas pointer.
 *
 * Returns 1 if the canvas was truncated, 0 otherwise.
 */
int canvas_trim(Canvas **canvas_pointer, int newrows, int newcols) {
  Canvas *orig = *canvas_pointer;
  Canvas *new = canvas_new(newrows, newcols);

  // copy over
  int res = canvas_ldcanvasyx(new, orig, 0, 0);

  // update and free
  *canvas_pointer = new;
  canvas_free(orig);
  return res;
}

/* Set a single character at position (x, y)
 *
 * Top left of canvas is (0, 0).
 */
void canvas_scharyx(Canvas *canvas, int y, int x, char c) {
  canvas->rows[y][x] = c;
}

/* Set a single character with single index i
 *
 * Index starts at 0 at position (0, 0) and increments first horizontally.
 */
void canvas_schari(Canvas *canvas, int i, char c) {
  int row = i / canvas->num_cols;
  int col = i % canvas->num_cols;
  canvas->rows[row][col] = c;
}

/* Get the character at position (x, y)
 *
 */
char canvas_gcharyx(Canvas *canvas, int y, int x) {
  assert(x <= canvas->num_cols);
  assert(y <= canvas->num_rows);
  return canvas->rows[y][x];
}

/* Get the character at index i
 *
 */
char canvas_gchari(Canvas *canvas, int i) {
  int row = i / canvas->num_cols;
  int col = i % canvas->num_cols;
  return canvas->rows[row][col];
}

/* Load str into canvas as point (x, y), ignoring char transparent.
 *
 * Newlines ('\n') cause the canvas to wrap to the beginning of the next line
 * (like in normal text).
 *
 * Stops when the canvas is full or it reaches the null character ('\0').
 *
 * Returns: the number of characters written
 *
 * TODO: write test_canvas_ldstryxc
 */
int canvas_ldstryxc(Canvas *canvas, char *str, int y, int x, char transparent) {
  int col = x;
  int row = y;
  int i;
  for (i = 0; str[i] != '\0'; i++) {
    // wrap to start col if at end
    if (col >= canvas->num_cols || str[i] == '\n') {
      row++;
      col = x;
      // skip newline chars
      if (str[i] == '\n') {
        continue;
      }
    }
    // finish if at end of canvas
    if (row >= canvas->num_rows) {
      break;
    }
    // update pixel value
    if (str[i] != transparent) {
      canvas_scharyx(canvas, row, col, str[i]);
    }
    col++;
  }
  return i;
}

/* Load str into canvas at (0, 0), ignoring no characters
 *
 * Newlines ('\n') cause the canvas to wrap to the beginning of the next line
 * (like in normal text).
 *
 * Stops when the canvas is full or it reaches the null character ('\0').
 *
 * Returns: the number of characters written
 */
int canvas_ldstryx(Canvas *canvas, char *str, int y, int x) {
  return canvas_ldstryxc(canvas, str, y, x, '\0');
}

/* Load str into canvas at (0, 0).
 *
 * Newlines ('\n') cause the canvas to wrap to the beginning of the next line
 * (like in normal text).
 *
 * Stops when the canvas is full or it reaches the null character ('\0').
 *
 * Returns: the number of characters written
 */
int canvas_ldstr(Canvas *canvas, char *str) {
  return canvas_ldstryx(canvas, str, 0, 0);
}

/* Duplicate of canvas_ldstr for backwards compatability.
 *
 * Newlines ('\n') cause the canvas to wrap to the beginning of the next line
 * (like in normal text).
 *
 * Stops when the canvas is full or it reaches the null character ('\0').
 *
 * Returns: the number of characters written
 */
int canvas_load_str(Canvas *canvas, char *str) {
  return canvas_ldstr(canvas, str);
}

/* Print a canvas to a file stream
 *
 * Returns: the number of characters printed if successful, or a negative value
 * on output error from fprintf
 */
int canvas_fprint(FILE *stream, Canvas *canvas) {
  char *row;
  int res;
  int total = 0;
  for (int i = 0; i < canvas->num_rows; i++) {
    // print row char by char
    row = canvas->rows[i];
    for (int j = 0; j < canvas->num_cols; j++) {
      res = fprintf(stream, "%c", row[j]);
      if (res < 0) {
        return res;
      }
      total += res;
    }
    // finish row
    res = fprintf(stream, "\n");
    if (res < 0) {
      return res;
    }
    total += res;
  }
  return total;
}

/* Print char c num times to stream.
 *
 * Returns: the number of characters printed if successful, or a negative value
 * on output error from fprintf
 */
int fprintcr(FILE *stream, char c, int num) {
  int total = 0;
  int res = 0;
  for (int i = 0; i < num; i++) {
    if ((res = fprintf(stream, "%c", c)) < 0) {
      return res;
    }
    total += res;
  }
  return total;
}

/* Print a canvas to a file stream, trimming trailing spaces.
 *
 * Returns: the number of characters printed if successful, or a negative value
 * on output error from fprintf
 */
int canvas_fprint_trim(FILE *stream, Canvas *canvas) {
  char *row;
  int res1, res2;
  int total = 0;
  int num_trailing_spaces = 0;
  for (int i = 0; i < canvas->num_rows; i++) {
    // print row char by char
    row = canvas->rows[i];
    for (int j = 0; j < canvas->num_cols; j++) {
      res1 = 0, res2 = 0;
      if (row[j] == ' ') {
        num_trailing_spaces++;
      } else {
        if (num_trailing_spaces > 0) {
          // print hoarded spaces if a non-whitespace character has appeared.
          res1 = fprintcr(stream, ' ', num_trailing_spaces);
          // reset counter
          num_trailing_spaces = 0;
        }
        res2 = fprintf(stream, "%c", row[j]);
      }
      if (res1 < 0) {
        return res1;
      } else if (res2 < 0) {
        return res2;
      }
      total += res1 + res2;
    }
    num_trailing_spaces = 0;
    // finish row
    // reuse res1
    res1 = fprintf(stream, "\n");
    if (res1 < 0) {
      return res1;
    }
    total += res1;
  }
  return total;
}

/* Print a canvas to stdout
 *
 * Returns: the number of characters printed if successful, or a negative
 * value on output error from fprintf
 */
int canvas_print(Canvas *canvas) { return canvas_fprint(stdout, canvas); }

/* Create a canvas from a text file object.
 *
 * Scans file to find dimensions, rewinds, and loads into canvas. Follows the
 * same behavior as `canvas_ldstr`, because that's what it uses internally.
 *
 * Returns a new Canvas.
 */
Canvas *canvas_readf(FILE *f) {
  int numlines = 0;
  int llength = 0;
  int maxllength = 0;
  int numchars = 0;

  int c;
  // read entire file, recording longest line and # lines
  while (1) {
    c = getc(f);
    if (c == EOF) {
      break;
    }
    numchars++;
    if (c == '\n') {
      // check if llength is larger
      if (llength > maxllength) {
        maxllength = llength;
      }
      // reset and continue
      llength = 0;
      numlines++;
    } else {
      llength++;
    }
  }
  // rewind to beginning
  rewind(f);
  // read file into char buffer
  int bufflen = numchars + 1;
  char buffer[bufflen];
  int i;
  for (i = 0; i < bufflen - 1; i++) {
    c = getc(f);
    if (c == EOF) {
      break;
    }
    buffer[i] = c;
  }
  // end buffer
  buffer[i] = '\0';
  // initialize canvas of a large enough size
  Canvas *canvas = canvas_new(numlines, maxllength);
  // load canvas from buffer
  canvas_ldstr(canvas, buffer);
  return canvas;
}

/* A variant of canvas_readf that doesn't use `rewind`, useful for streams.
 *
 * Rather than rewind, it does a single pass of the file, dynamically
 * reallocating a buffer and tracking lines/line endings as it reads. Follows
 * the same behavior as `canvas_ldstr`, because that's what it uses internally.
 *
 * Returns a new Canvas.
 */
Canvas *canvas_readf_norewind(FILE *f) {
  int numlines = 0;
  int llength = 0;
  int maxllength = 0;
  size_t numchars = 0;

  size_t BUFFSIZE = 1024;
  char *buffer;
  if ((buffer = malloc(BUFFSIZE)) == NULL) {
    perror("canvas_readf_norewind malloc");
    exit(1);
  }

  int c;
  // read entire file, recording longest line and # lines
  while (1) {
    if (numchars >= BUFFSIZE - 1) {
      // reallocate
      BUFFSIZE *= 2;
      if ((buffer = realloc(buffer, BUFFSIZE * sizeof(char))) == NULL) {
        perror("canvas_readf_norewind realloc");
        exit(1);
      }
      logd("Resized readf buffer to %li\n", BUFFSIZE);
    }
    c = getc(f);
    // stop at EOF
    if (c == EOF) {
      buffer[numchars] = '\0';
      break;
    }

    // add char to buffer
    buffer[numchars] = c;
    numchars++;

    // track lines and line length
    if (c == '\n') {
      // check if llength is larger
      if (llength > maxllength) {
        maxllength = llength;
      }
      // reset and continue
      llength = 0;
      numlines++;
    } else {
      llength++;
    }
  }

  // initialize canvas of a large enough size
  Canvas *canvas = canvas_new(numlines, maxllength);
  // load canvas from buffer
  canvas_ldstr(canvas, buffer);
  free(buffer);
  return canvas;
}

/* Convert a canvas object into a character buffer
 *
 * A canvas of size n rols, m cols requires a buffer of size n*m bytes
 * (chars).
 *
 * Does NOT null-terminate the buffer.
 *
 * TODO: accept sizeof(buf) and assert size is correct?
 *
 * Returns: the number of bytes written to buf
 */
int canvas_serialize(Canvas *canvas, char *buf) {
  int i;
  for (i = 0; i < canvas->num_cols * canvas->num_rows; i++) {
    buf[i] = canvas_gchari(canvas, i);
  }
  return i;
}

/* Load a serialized canvas into a canvas object
 *
 * TODO: consider creating a canvas instead of filling one
 *
 * TODO: get size of buffer?
 */
void canvas_deserialize(char *bytes, Canvas *canvas) {
  canvas_load_str(canvas, bytes);
}

/* Check if two canvases are the same
 *
 * Returns: 1 if equal, 0 if not
 */
int canvas_eq(Canvas *a, Canvas *b) {
  // compare sizes
  if (a->num_cols != b->num_cols || a->num_rows != b->num_rows) {
    return 0;
  }
  // compare values
  int size = a->num_cols * a->num_rows;
  for (int i = 0; i < size; i++) {
    if (canvas_gchari(a, i) != canvas_gchari(b, i)) {
      return 0;
    }
  }
  // return 1 if both pass
  return 1;
}
