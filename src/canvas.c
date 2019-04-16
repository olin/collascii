/* API for Canvas objects
 *
 * TODO: add save/read from file options
 * TODO: assert get/set positions are within canvas sizes?
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "canvas.h"

/* Create a canvas object
 *
 * Returned pointer should be freed with free_canvas
 */
Canvas *canvas_new(int rows, int cols)
{
  Canvas *canvas = malloc(sizeof(Canvas));
  canvas->num_cols = cols;
  canvas->num_rows = rows;
  canvas->rows = malloc(rows * sizeof(char *));
  for (int i = 0; i < rows; i++)
  {
    canvas->rows[i] = calloc(cols, sizeof(char));
  }
  return canvas;
}

Canvas *canvas_new_blank(int rows, int cols)
{
  Canvas *canvas = canvas_new(rows, cols);
  for (int i = 0; i < (rows * cols); i++)
  {
    canvas_schari(canvas, i, ' ');
  }
  return canvas;
}

/* Create and return a deep copy of a canvas
 *
 * Returned pointer should be freed with free_canvas
 */
Canvas *canvas_cpy(Canvas *orig)
{
  // allocate new canvas
  Canvas *copy = canvas_new(orig->num_rows, orig->num_cols);
  // copy rows over from orig
  for (int i = 0; i < orig->num_rows; i++)
  {
    memcpy(copy->rows[i], orig->rows[i], sizeof(char) * orig->num_cols);
  }

  return copy;
}

/* Free a canvas object
 *
 */
void canvas_free(Canvas *canvas)
{
  // free each row first
  for (int i = 0; i < canvas->num_rows; i++)
  {
    free(canvas->rows[i]);
  }
  // free array of rows
  free(canvas->rows);
  // free struct itself
  free(canvas);
}

/* Set a single character at position (x, y)
 *
 * Top left of canvas is (0, 0).
 */
void canvas_scharyx(Canvas *canvas, int y, int x, char c)
{
  canvas->rows[y][x] = c;
}

/* Set a single character with single index i
 *
 * Index starts at 0 at position (0, 0) and increments first horizontally.
 */
void canvas_schari(Canvas *canvas, int i, char c)
{
  int row = i / canvas->num_cols;
  int col = i % canvas->num_cols;
  canvas->rows[row][col] = c;
}

/* Get the character at position (x, y)
 *
 */
char canvas_gcharyx(Canvas *canvas, int y, int x) { return canvas->rows[y][x]; }

/* Get the character at index i
 *
 */
char canvas_gchari(Canvas *canvas, int i)
{
  int row = i / canvas->num_cols;
  int col = i % canvas->num_cols;
  return canvas->rows[row][col];
}

/* Fill a canvas with characters from string str
 *
 * Stops at the null character or when the canvas is full.
 *
 * String indices are mapped to canvas indices, so with a canvas of size 3x3
 * the first character is placed at (0, 0), the second at (1, 0), and the fourth
 * at (1, 0).
 *
 * Returns: the number of characters written
 */
int canvas_load_str(Canvas *canvas, char *str)
{
  int i;
  for (i = 0; str[i] != '\0' && i < canvas->num_cols * canvas->num_rows; i++)
  {
    canvas_schari(canvas, i, str[i]);
  }
  return i;
}

/* Print a canvas to a file stream
 *
 * Returns: the number of characters printed if successful, or a negative value
 * on output error from fprintf
 */
int canvas_fprint(FILE *stream, Canvas *canvas)
{
  char *row;
  int res;
  int total = 0;
  for (int i = 0; i < canvas->num_rows; i++)
  {
    // print row char by char
    row = canvas->rows[i];
    for (int j = 0; j < canvas->num_cols; j++)
    {
      res = fprintf(stream, "%c", row[j]);
      if (res < 0)
      {
        return res;
      }
      total += res;
    }
    // finish row
    res = printf("\n");
    if (res < 0)
    {
      return res;
    }
    total += res;
  }
  return total;
}

/* Print a canvas to stdout
 *
 * Returns: the number of characters printed if successful, or a negative value
 * on output error from fprintf
 */
int canvas_print(Canvas *canvas) { return canvas_fprint(stdout, canvas); }

/* Convert a canvas object into a character buffer
 *
 * A canvas of size n rols, m cols requires a buffer of size n*m bytes (chars).
 *
 * Does NOT null-terminate the buffer.
 *
 * TODO: accept sizeof(buf) and assert size is correct?
 *
 * Returns: the number of bytes written to buf
 */
int canvas_serialize(Canvas *canvas, char *buf)
{
  int i;
  for (i = 0; i < canvas->num_cols * canvas->num_rows; i++)
  {
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
void canvas_deserialize(char *bytes, Canvas *canvas)
{
  canvas_load_str(canvas, bytes);
}

/* Check if two canvases are the same
 *
 * Returns: 1 if equal, 0 if not
 */
int canvas_eq(Canvas *a, Canvas *b)
{
  // compare sizes
  if (a->num_cols != b->num_cols || a->num_rows != b->num_rows)
  {
    return 0;
  }
  // compare values
  int size = a->num_cols * a->num_rows;
  for (int i = 0; i < size; i++)
  {
    if (canvas_gchari(a, i) != canvas_gchari(b, i))
    {
      return 0;
    }
  }
  // return 1 if both pass
  return 1;
}
