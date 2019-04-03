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

int load_string(Canvas *canvas, char *str)
{
    int slen = strlen(str);
    for (int i = 0; i < slen; i++)
    {
        schari(canvas, i, str[i]);
    }
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

int main() {
    Canvas* c = make_canvas(3, 3);
    load_string(c, "X XXXXX X");
    print_canvas(c);
    printf("\n");
    scharxy(c, 1, 2, 'O');
    print_canvas(c);
    free_canvas(c);
    return 0;
}