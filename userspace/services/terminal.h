/* terminal.h */

#ifndef TERMINAL_1
#define TERMINAL_1

#include "compositor.h"
#include <common/types.h>

typedef struct Terminal {
    int cols;
    int rows;
    int cursor_x;
    int cursor_y;
    uint32 fg;
    uint32 bg;
    Surface* surface;
} Terminal;

void terminal_draw_cursor(Terminal* t);
void terminal_put_char(Terminal* t, char c);
void terminal_render(Terminal* t);
void terminal_init(Terminal* t, Surface* s, int row_count, int col_count, uint32 fg, uint32 bg);

#endif
#ifdef IMPL_TERMINAL_1
#undef IMPL_TERMINAL_1

#define IMPL_FONT_1
#include <common/heap.h>
#include <hal/font.h>

void terminal_draw_cursor(Terminal* t)
{
    if (!t || !t->surface)
        return;

    int px = t->cursor_x * 8;
    int py = t->cursor_y * 16 + 15;

    for (int i = 0; i < 8; i++)
        surface_put_pixel(t->surface, px + i, py, t->fg);
}

void terminal_clear(Terminal* t)
{
    if (!t || !t->surface)
        return;

    surface_clear(t->surface, t->bg);

    t->cursor_x = 0;
    t->cursor_y = 0;
}

void terminal_scroll(Terminal* t)
{
    if (!t || !t->surface)
        return;

    int row_px    = 16;
    int width_px  = t->cols * 8;
    int height_px = t->rows * 16;

    surface_blit(t->surface, 0, row_px, 0, 0, width_px, height_px - row_px);
    surface_fill_rect(t->surface, 0, height_px - row_px, width_px, row_px, t->bg);
}

void terminal_put_char(Terminal* t, char c)
{
    if (!t || !t->surface)
        return;

    surface_draw_char(t->surface, ' ', t->cursor_x * 8, t->cursor_y * 16, t->fg, t->bg);

    if (c == '\n') {
        t->cursor_x = 0;
        t->cursor_y += 1;
    } else if (c == '\r') {
        t->cursor_x = 0;
    } else if (c == '\b') {
        if (t->cursor_x > 0)
            t->cursor_x -= 1;
        surface_draw_char(t->surface, ' ', t->cursor_x * 8, t->cursor_y * 16, t->fg, t->bg);
    } else {
        surface_draw_char(t->surface, c, t->cursor_x * 8, t->cursor_y * 16, t->fg, t->bg);
        t->cursor_x += 1;
    }

    if (t->cursor_x >= t->cols) {
        t->cursor_x = 0;
        t->cursor_y += 1;
    }

    if (t->cursor_y >= t->rows) {
        terminal_scroll(t);
        t->cursor_y = t->rows - 1;
    }
}

void terminal_render(Terminal* t)
{
    if (!t || !t->surface)
        return;

    terminal_draw_cursor(t);
}

void terminal_init(Terminal* t, Surface* s, int row, int col, uint32 fg, uint32 bg)
{
    if (!t || !s)
        return;

    t->rows    = row;
    t->cols    = col;

    t->fg      = fg;
    t->bg      = bg;

    t->surface = s;

    terminal_clear(t);
}

#endif
