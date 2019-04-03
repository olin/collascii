#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct
{
    int num_cols, num_rows;
    char **rows;
} Canvas;

Canvas *make_canvas(int cols, int rows)
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

void free_canvas(Canvas *canvas)
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
 * Top left of canvas is (0, 0)
 */
void scharxy(Canvas *canvas, int x, int y, char c)
{
    canvas->rows[y][x] = c;
}

void schari(Canvas *canvas, int i, char c)
{
    int row = i / canvas->num_cols;
    int col = i % canvas->num_cols;
    scharxy(canvas, col, row, c);
}

char gcharxy(Canvas *canvas, int x, int y) {
    return canvas->rows[y][x];
}

char gchari(Canvas *canvas, int i) {
    int row = i / canvas->num_cols;
    int col = i % canvas->num_cols;
    return canvas->rows[row][col];
}

int load_string(Canvas *canvas, char *str)
{
    int i;
    for (i = 0; str[i] != '\0' && i < canvas->num_cols*canvas->num_rows; i++)
    {
        schari(canvas, i, str[i]);
    }
    return i;
}

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

int serialize_canvas(Canvas *canvas, char* buf) {
    int i;
    for (i = 0; i < canvas->num_cols*canvas->num_rows; i++){
        buf[i] = gchari(canvas, i);
    }
    return i;
}

void deserialize_canvas(char* bytes, Canvas* canvas) {
    load_string(canvas, bytes);
}

// int main() {
//     // creating
//     Canvas* c = make_canvas(3, 3);
//     load_string(c, "X XXXXX X");
//     print_canvas(c);

//     // setting
//     printf("Set (1, 2) to 'O'\n");
//     scharxy(c, 1, 2, 'O');
//     print_canvas(c);

//     // serialization
//     printf("Original:\n");
//     print_canvas(c);
//     char buffer[10];
//     serialize_canvas(c, buffer);
//     buffer[9] = '\0';
//     printf("Serialized:\n");
//     printf("'%s'\n", buffer);
//     Canvas* c2 = make_canvas(3,3);
//     deserialize_canvas(buffer, c2);
//     printf("Deserialized:\n");
//     print_canvas(c2);

//     // free
//     free_canvas(c);
//     return 0;
// }