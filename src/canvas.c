/* API for Canvas objects
 *
 * TODO: add function to check if canvases are equal
 * TODO: add save/read from file options
 * TODO: conform to ncurses y,x
 * TODO: prepend canvas on canvas functions
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct
{
    int num_cols, num_rows;
    char **rows;
} Canvas;

/* Create a canvas object
 *
 * TODO: swap to rows, cols to match ncurses height, width format
 *
 * Returned pointer should be freed with free_canvas
 */
Canvas *canvas_new(int cols, int rows)
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

/* Set a single character at position x, y
 *
 * Top left of canvas is (0, 0).
 */
void scharxy(Canvas *canvas, int x, int y, char c)
{
    canvas->rows[y][x] = c;
}

/* Set a single character with single index i
 *
 * Index starts at 0 at position (0, 0) and increments first horizontally.
 */
void schari(Canvas *canvas, int i, char c)
{
    int row = i / canvas->num_cols;
    int col = i % canvas->num_cols;
    scharxy(canvas, col, row, c);
}

/* Get the character at position x, y
 *
 */
char gcharxy(Canvas *canvas, int x, int y) {
    return canvas->rows[y][x];
}

/* Get the character at index i
 *
 */
char gchari(Canvas *canvas, int i) {
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
int load_string(Canvas *canvas, char *str)
{
    int i;
    for (i = 0; str[i] != '\0' && i < canvas->num_cols*canvas->num_rows; i++)
    {
        schari(canvas, i, str[i]);
    }
    return i;
}

/* Print a canvas to stdout
 *
 * TODO: add option ala fprintf for stderr
 */
void print_canvas(Canvas *canvas)
{
    char *row;
    for (int i = 0; i < canvas->num_rows; i++)
    {
        row = canvas->rows[i];
        for (int j = 0; j < canvas->num_cols; j++)
        {
            printf("%c", row[j]);
        }
        printf("\n");
    }
}

/* Convert a canvas object into a character buffer
 *
 * A canvas of size nxm requires a buffer of size n*m bytes (chars).
 *
 * Does NOT null-terminate the buffer.
 *
 * TODO: accept sizeof(buf) and assert size is correct?
 *
 * Returns: the number of bytes written to buf
 */
int serialize_canvas(Canvas *canvas, char* buf) {
    int i;
    for (i = 0; i < canvas->num_cols*canvas->num_rows; i++){
        buf[i] = gchari(canvas, i);
    }
    return i;
}

/* Load a serialized canvas into a canvas object
 *
 * TODO: consider creating a canvas instead of filling one
 */
void deserialize_canvas(char* bytes, Canvas* canvas) {
    load_string(canvas, bytes);
}

int main() {
    // creating
    Canvas* c = canvas_new(3, 3);
    load_string(c, "X XXXXX X");
    print_canvas(c);

    // setting
    printf("Set (1, 2) to 'O'\n");
    scharxy(c, 1, 2, 'O');
    print_canvas(c);

    // serialization
    printf("Original:\n");
    print_canvas(c);
    char buffer[10];
    serialize_canvas(c, buffer);
    buffer[9] = '\0';
    printf("Serialized:\n");
    printf("'%s'\n", buffer);
    Canvas* c2 = canvas_new(3,3);
    deserialize_canvas(buffer, c2);
    printf("Deserialized:\n");
    print_canvas(c2);

    // free
    canvas_free(c);
    canvas_free(c2);
    return 0;
}