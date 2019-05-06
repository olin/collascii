#ifndef canvas_h
#define canvas_h

#include <stdio.h>

typedef struct {
  int num_cols, num_rows;
  char **rows;
} Canvas;

Canvas *canvas_new(int rows, int cols);
Canvas *canvas_new_blank(int rows, int cols);
Canvas *canvas_cpy(Canvas *orig);
Canvas *canvas_cpy_p1p2(Canvas *orig, int y1, int x1, int y2, int x2);
void canvas_free(Canvas *canvas);

int canvas_isin_y(Canvas *canvas, int y);
int canvas_isin_x(Canvas *canvas, int x);
int canvas_isin_yx(Canvas *canvas, int y, int x);
int canvas_isin_i(Canvas *canvas, int i);
int canvas_eq(Canvas *a, Canvas *b);

void canvas_scharyx(Canvas *canvas, int y, int x, char c);
void canvas_schari(Canvas *canvas, int i, char c);
char canvas_gcharyx(Canvas *canvas, int y, int x);
char canvas_gchari(Canvas *canvas, int i);

int canvas_ldcanvasyx(Canvas *dest, Canvas *source, int y, int x);
int canvas_ldcanvasyxc(Canvas *dest, Canvas *source, int y, int x,
                       char transparent);
int canvas_resize(Canvas **orig, int newrows, int newcols);

int canvas_ldstryx(Canvas *canvas, char *str, int y, int x);
int canvas_ldstr(Canvas *canvas, char *str);
int canvas_load_str(Canvas *canvas, char *str);

Canvas *canvas_readf(FILE *f);
Canvas *canvas_readf_norewind(FILE *f);
int canvas_fprint(FILE *stream, Canvas *canvas);
int canvas_fprint_trim(FILE *stream, Canvas *canvas);
int canvas_print(Canvas *canvas);

int canvas_serialize(Canvas *canvas, char *buf);
void canvas_deserialize(char *bytes, Canvas *canvas);

#endif
