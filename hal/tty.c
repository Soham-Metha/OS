#include "hal.h"
#include "interrupt.h"

tty* active_tty = (void*)0;

void draw_cursor(Terminal* t)
{
    int px = t->cursor_x * 8;
    int py = t->cursor_y * 16 + 15;

    for (int i = 0; i < 8; i++)
        hal_put_pixel(px + i, py, t->fg);
}

void tty_init(struct tty* t, int width, int height, uint32 fg, uint32 bg)
{
    if (!t)
        return;

    terminal_init(&t->term, height, width, fg, bg);

    t->head    = 0;
    t->tail    = 0;
    t->echo    = true;

    active_tty = t;
    draw_cursor(&t->term);
    hal_present();
}

void tty_push_key(tty* t, uint8 c)
{
    t->buf[t->head] = c;
    t->head         = (t->head + 1) % TTY_BUF_SIZE;

    if (t->echo) {
        tty_write_char(t, c);
        tty_flush(t);
    }
}

uint8 tty_read_char(tty* t)
{
    if (t->head == t->tail)
        return -1;
    char c  = t->buf[t->tail];
    t->tail = (t->tail + 1) % TTY_BUF_SIZE;
    return c;
}

void tty_write_char(tty* t, char c)
{
    terminal_put_char(&t->term, c);
    if (c == '\n')
        tty_flush(t);
}

void tty_flush(tty* t)
{
    terminal_render(&t->term);
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

    draw_cursor(t);
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
