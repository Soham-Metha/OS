/* terminal.h */

#ifndef TERMINAL_1
#define TERMINAL_1

#include <common/types.h>

struct cell {
    char ch;
    uint32 fg;
    uint32 bg;
};

typedef struct Terminal {
    struct cell buffer[65535];
    int cols;
    int rows;
    int cursor_x;
    int cursor_y;
    uint32 fg;
    uint32 bg;
} Terminal;

#define IDX(t, x, y) (y * t->cols + x)

void terminal_draw_cursor(Terminal* t);
void terminal_clear(Terminal* t);
void terminal_put_char(Terminal* t, char c);
void terminal_render(Terminal* t);
void terminal_init(Terminal* t, int row, int col, uint32 fg, uint32 bg);

#endif
#ifdef IMPL_TERMINAL_1

#define IMPL_FONT_1
#include <hal/font.h>
#include <hal/hal.h>

void terminal_draw_cursor(Terminal* t)
{
    int px = t->cursor_x * 8;
    int py = t->cursor_y * 16 + 15;

    for (int i = 0; i < 8; i++)
        hal_put_pixel(px + i, py, t->fg);
}

void terminal_clear(Terminal* t)
{
    for (int y = 0; y < t->rows; y++)
        for (int x = 0; x < t->cols; x++)
            t->buffer[IDX(t, x, y)] = (struct cell) { ' ', t->fg, t->bg };

    t->cursor_x = 0;
    t->cursor_y = 0;
}

void terminal_scroll(Terminal* t)
{
    for (int y = 1; y < t->rows; y++) {
        for (int x = 0; x < t->cols; x++) {
            t->buffer[IDX(t, x, y - 1)] = t->buffer[IDX(t, x, y)];
        }
    }

    for (int x = 0; x < t->cols; x++)
        t->buffer[IDX(t, x, t->rows - 1)] = (struct cell) { ' ', t->fg, t->bg };
}

void terminal_put_char(Terminal* t, char c)
{
    if (c == '\n') {
        t->cursor_x = 0;
        t->cursor_y += 1;
    } else if (c == '\r') {
        t->cursor_x = 0;
    } else if (c == '\b') {
        if (t->cursor_x > 0)
            t->cursor_x -= 1;
        t->buffer[IDX(t, t->cursor_x, t->cursor_y)].ch = ' ';
    } else {
        t->buffer[IDX(t, t->cursor_x, t->cursor_y)] = (struct cell) {
            .ch = c,
            .fg = t->fg,
            .bg = t->bg
        };
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
    for (int y = 0; y < t->rows; y++) {
        for (int x = 0; x < t->cols; x++) {
            struct cell* c = &t->buffer[IDX(t, x, y)];
            draw_char(c->ch, x * 8, y * 16, c->fg, c->bg);
        }
    }

    terminal_draw_cursor(t);
    hal_present();
}

void terminal_init(Terminal* t, int row, int col, uint32 fg, uint32 bg)
{
    if (!t)
        return;
    t->rows = row;
    t->cols = col;

    t->fg   = fg;
    t->bg   = bg;

    terminal_clear(t);
}

#endif
