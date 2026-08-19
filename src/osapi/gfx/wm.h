/*
 * wm.h
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
#ifndef WM_1
#define WM_1

#include "compositor.h"
#include "terminal.h"
#include <common/event.h>
#include <common/memmanager.h>
#include <common/types.h>

#define WM_MAX_WINDOWS 4

typedef struct Window {
    Arena arena;
    Surface surface;
    Terminal term;
    bool focused;
    bool visible;
} Window;

typedef struct WindowManager {
    Window windows[WM_MAX_WINDOWS];
    int mx;
    int my;
    int count;
    int focused;
    Compositor* compositor;
} WindowManager;

void wm_init(WindowManager* wm, Compositor* c);

Window* wm_create_window(WindowManager* wm, int x, int y, int w, int h, uint32 fg, uint32 bg);
void wm_destroy_window(WindowManager* wm, Window* win);
void wm_focus_window(WindowManager* wm, Window* win);

void wm_handle_key(WindowManager* wm, uint8 key);
void wm_handle_mouse(WindowManager* wm, MouseEvent me);

void wm_render(WindowManager* wm);

#endif
#ifdef IMPL_WM_1
#undef IMPL_WM_1

#define IMPL_COMPOSITOR_1
#include "compositor.h"
#include "font.h"

void wm_init(WindowManager* wm, Compositor* c)
{
    wm->count      = 0;
    wm->focused    = -1;
    wm->mx         = 0;
    wm->my         = 0;
    wm->compositor = c;

    for (int i = 0; i < WM_MAX_WINDOWS; i++) {
        wm->windows[i].visible = false;
        wm->windows[i].focused = false;
    }
}

Window* wm_create_window(WindowManager* wm, int x, int y, int w, int h, uint32 fg, uint32 bg)
{
    if (wm->count >= WM_MAX_WINDOWS)
        return 0;

    Window* win     = &wm->windows[wm->count++];
    ResultPtr space = region_alloc(&win->arena, w * h * sizeof(uint32));
    try(RESULT_OK(space), "out of space!", "");
    win->surface.x       = x;
    win->surface.y       = y;
    win->surface.canvas  = gfx_init_canvas((uint32*)RESULT_VAL(space), w, h, 1.0f);
    win->surface.visible = true;
    win->surface.dirty   = true;
    win->visible         = true;

    compositor_attach(wm->compositor, &win->surface);
    terminal_init(&win->term, &win->surface, h / GLYPH_H, w / GLYPH_W, fg, bg);
    wm_focus_window(wm, win);

    return win;
ret_err:
    return (Window*)0;
}

void wm_focus_window(WindowManager* wm, Window* win)
{
    for (int i = 0; i < wm->count; i++)
        wm->windows[i].focused = false;

    win->focused = true;

    for (int i = 0; i < wm->count; i++) {
        if (&wm->windows[i] == win) {
            wm->focused = i;
            compositor_raise(wm->compositor, &win->surface);
            break;
        }
    }
}

void wm_handle_key(WindowManager* wm, uint8 key)
{
    if (wm->focused >= 0) {
        terminal_put_char(&wm->windows[wm->focused].term, key);
        terminal_draw_cursor(&wm->windows[wm->focused].term);
    }
}

void surface_swap_colors_at(Surface* s, int x, int y, uint32 fg, uint32 bg)
{
    if (!s)
        return;
    for (int row = 0; row < GLYPH_H; row++) {
        for (int col = 0; col < GLYPH_W; col++) {
            uint32 *px = &s->canvas.px[(y + row) * s->canvas.px_stride + (x + col)];
            *px = (*px == bg) ? fg : bg;
        }
    }
    s->dirty = true;
}

void wm_handle_mouse(WindowManager* wm, MouseEvent me)
{
    static int cursor_win = -1;

    if (!wm || !wm->compositor || wm->count == 0)
        return;

    int newx = wm->mx + me.dx;
    int newy = wm->my + me.dy;

    if (newx < 0)
        newx = 0;
    if (newx >= wm->compositor->width)
        newx = wm->compositor->width - 1;
    if (newy < 0)
        newy = 0;
    if (newy >= wm->compositor->height)
        newy = wm->compositor->height - 1;

    if (wm->mx == newx && wm->my == newy && !me.left) {
        return;
    }

    if (cursor_win >= 0 && cursor_win < wm->count) {
        surface_swap_colors_at(&wm->windows[cursor_win].surface,
            wm->mx - wm->windows[cursor_win].surface.x,
            wm->my - wm->windows[cursor_win].surface.y,
            wm->windows[cursor_win].term.fg,
            wm->windows[cursor_win].term.bg);
    }

    wm->mx = newx;
    wm->my = newy;

    // TODO: optimization(z buffering?)
    for (int i = wm->count - 1; i >= 0; i--) {
        Surface* s = &wm->windows[i].surface;

        int lx = newx - s->x;
        int ly = newy - s->y;

        if (lx >= 0 && lx < s->canvas.px_w &&
            ly >= 0 && ly < s->canvas.px_h)
        {
            surface_swap_colors_at(s, lx, ly, wm->windows[i].term.fg, wm->windows[i].term.bg);
            wm_focus_window(wm, &wm->windows[i]);
            cursor_win = i;
            return;
        }
    }
}

void wm_render(WindowManager* wm)
{
    compositor_render(wm->compositor);
}

#else
extern WindowManager wm;
#endif
