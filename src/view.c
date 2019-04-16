#include "view.h"

View *view_new(Canvas *canvas)
{
    View *view = malloc(sizeof(View));
    view->x = 0;
    view->y = 0;
    view->canvas = canvas;
    return view;
}

View *view_new_startpos(Canvas *canvas, int x, int y)
{
    View *view = view_new(canvas);
    view->x = x;
    view->y = y;
    return view;
}

void view_move_up(View *view)
{
    if (view->y == 0)
    {
        return;
    }
    view->y--;
}

void view_move_down(View *view)
{
    if (view->y - view_max_y == view->canvas->num_rows)
    { // Take box lines into account
        return;
    }
    view->y++;
}

void view_move_left(View *view)
{
    if (view->x == 0)
    {
        return;
    }
    view->x--;
}

void view_move_right(View *view)
{
    if (view->x + view_max_x == view->canvas->num_cols)
    { // Take box lines into account
        return;
    }
    view->x++;
}

void view_pan_ch(int arrow, View *view)
{
    switch (arrow)
    {
    case KEY_LEFT:
        view_move_left(view);
        break;
    case KEY_RIGHT:
        view_move_right(view);
        break;
    case KEY_UP:
        view_move_up(view);
        break;
    case KEY_DOWN:
        view_move_down(view);
        break;
    }
}

void view_free(View *view)
{
    free(view);
}
