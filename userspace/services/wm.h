/* wm.h */

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
    wm->count        = 0;
    wm->focused      = -1;
    wm->mx           = 0;
    wm->my           = 0;
    wm->compositor   = c;

    for (int i = 0; i < WM_MAX_WINDOWS; i++) {
        wm->windows[i].visible = false;
        wm->windows[i].focused = false;
    }
}

Window* wm_create_window(WindowManager* wm, int x, int y, int w, int h, uint32 fg, uint32 bg)
{
    if (wm->count >= WM_MAX_WINDOWS)
        return 0;

    Window* win          = &wm->windows[wm->count++];
    win->surface.x       = x;
    win->surface.y       = y;
    win->surface.width   = w;
    win->surface.height  = h;
    win->surface.pixels  = region_alloc(&win->arena, w * h * sizeof(uint32));
    win->surface.visible = true;
    win->surface.dirty   = true;
    win->visible         = true;

    compositor_attach(wm->compositor, &win->surface);
    terminal_init(&win->term, &win->surface, h / GLYPH_H, w / GLYPH_W, fg, bg);
    wm_focus_window(wm, win);

    return win;
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
    for (int row = 0; row < GLYPH_H; row++) {
        for (int col = 0; col < GLYPH_W; col++) {
            uint32 color = (s->pixels[SURF_IDX(s, x + col, y + row)] == bg) ? fg : bg;

            surface_put_pixel(s, x + col, y + row, color);
        }
    }
}

void wm_handle_mouse(WindowManager* wm, MouseEvent me)
{
    static int cursor_win = -1;
    if (!wm || !wm->count || !wm->windows[0].surface.pixels)
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

    if (cursor_win >= 0) {
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
        if ((newx - wm->windows[i].surface.x) >= 0 && (newx - wm->windows[i].surface.x) < wm->windows[i].surface.width
            && (newy - wm->windows[i].surface.y) >= 0 && (newy - wm->windows[i].surface.y) < wm->windows[i].surface.height) {
            surface_swap_colors_at(&wm->windows[i].surface,
                newx - wm->windows[i].surface.x,
                newy - wm->windows[i].surface.y,
                wm->windows[i].term.fg,
                wm->windows[i].term.bg);
            if (me.left) {
                wm->windows[wm->focused].focused = false;
                wm->focused                      = i;
                wm->windows[wm->focused].focused = true;
            }
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
