#ifndef canvas_h
#define canvas_h

#include <stdio.h>

typedef struct
{
    int num_cols, num_rows;
    char **rows;
} Canvas;

Canvas *canvas_new(int rows, int cols);
Canvas *canvas_cpy(Canvas *orig);
void canvas_free(Canvas *canvas);
void canvas_scharxy(Canvas *canvas, int x, int y, char c);
void canvas_schari(Canvas *canvas, int i, char c);
char canvas_gcharxy(Canvas *canvas, int x, int y);
char canvas_gchari(Canvas *canvas, int i);
int canvas_load_str(Canvas *canvas, char *str);
int canvas_fprint(FILE *stream, Canvas *canvas);
int canvas_print(Canvas *canvas);
int canvas_serialize(Canvas *canvas, char *buf);
void canvas_deserialize(char *bytes, Canvas *canvas);
int canvas_eq(Canvas *a, Canvas *b);

#endif