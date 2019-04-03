#ifndef canvas_h
#define canvas_h

typedef struct
{
    int num_cols, num_rows;
    char **rows;
} Canvas;

Canvas *make_canvas(int cols, int rows);
void free_canvas(Canvas *canvas);
void scharxy(Canvas *canvas, int x, int y, char c);
void schari(Canvas *canvas, int i, char c);
char gcharxy(Canvas *canvas, int x, int y);
char gchari(Canvas *canvas, int i);
int load_string(Canvas *canvas, char *str);
void print_canvas(Canvas *canvas);
void deserialize_canvas(char* bytes, Canvas* canvas);
int serialize_canvas(Canvas *canvas, char* buf);


#endif