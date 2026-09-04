/*
 * terminal.h
 *  Copyright (C) 2026 Soham Metha
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
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
    Font font;
    Surface* surface;
} Terminal;

void terminal_clear(Terminal* t);
void terminal_put_char(Terminal* t, char c);
void terminal_init(Terminal* t, Surface* s, Font f, int row_count, int col_count, uint32 fg, uint32 bg);

#endif
#ifdef IMPL_TERMINAL_1
#undef IMPL_TERMINAL_1

#define IMPL_FONT_1
#include <common/font.h>
#include <kernel/heap.h>

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

    int row_height  = t->font.cell_h;
    int term_width  = t->cols * t->font.cell_w;
    int term_height = t->rows * t->font.cell_h;

    surface_copy_rect(t->surface, 
        0, row_height,
        0, 0, 
        term_width, (term_height - row_height)
    );
    surface_fill_rect(t->surface, 
        0, (term_height - row_height),
        term_width, row_height, 
        t->bg
    );
}

void terminal_put_char(Terminal* t, char c)
{
    if (!t || !t->surface)
        return;

    surface_draw_char(t->surface, t->font, ' ', t->cursor_x, t->cursor_y, t->fg, t->bg);     // erase cursor

    if (c == '\n') {
        t->cursor_x = 0;
        t->cursor_y += 1;
    } else if (c == '\r') {
        t->cursor_x = 0;
    } else if (c == '\b') {
        if (t->cursor_x > 0)
            t->cursor_x -= 1;
    } else {
        surface_draw_char(t->surface, t->font, c, t->cursor_x, t->cursor_y, t->fg, t->bg);
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

    surface_draw_char(t->surface, t->font, '_', t->cursor_x, t->cursor_y, t->fg, t->bg);     // draw cursor
}

void terminal_init(Terminal* t, Surface* s, Font f, int row, int col, uint32 fg, uint32 bg)
{
    if (!t || !s)
        return;

    t->font    = f;
    t->rows    = row;
    t->cols    = col;

    t->fg      = fg;
    t->bg      = bg;

    t->surface = s;

    terminal_clear(t);
}

#endif
