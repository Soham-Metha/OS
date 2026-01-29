/* wm.h */

#ifndef WM_1
#define WM_1

#include "compositor.h"
#include "terminal.h"
#include <common/types.h>
#include <kernel/event.h> // TODO: fix boundary violation

#define WINDOW_MAX_W 800
#define WINDOW_MAX_H 600
#define WM_MAX_WINDOWS 3

typedef struct tty tty;

typedef struct Window {
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
    tty* fallback_tty;
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
#include <common/heap.h>
#include <drivers/tty.h>

void wm_init(WindowManager* wm, Compositor* c)
{
    wm->count        = 0;
    wm->focused      = -1;
    wm->mx           = 0;
    wm->my           = 0;
    wm->compositor   = c;
    wm->fallback_tty = (void*)0;

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
    win->surface.pixels  = kmalloc(w * h * sizeof(uint32));
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
    } else {
        tty_push_key(wm->fallback_tty, key);
    }
}

void wm_handle_mouse(WindowManager* wm, MouseEvent me)
{
    if (!wm || !wm->count || !wm->windows[0].surface.pixels)
        return;

    int newx = wm->mx + me.dx;
    int newy = wm->my + me.dy;

    if (newx < 0)
        newx = 0;
    if (newx >= wm->windows[0].surface.width)
        newx = wm->windows[0].surface.width - 1;
    if (newy < 0)
        newy = 0;
    if (newy >= wm->windows[0].surface.height)
        newy = wm->windows[0].surface.height - 1;

    if (wm->mx != newx || wm->my != newy) {
        wm->mx = newx;
        wm->my = newy;
        /* Horizontal line */
        for (int i = -4; i <= 4; i++) {
            surface_put_pixel(&wm->windows[0].surface, wm->mx + i, wm->my, 0x000000FF);
        }

        /* Vertical line */
        for (int i = -4; i <= 4; i++) {
            surface_put_pixel(&wm->windows[0].surface, wm->mx, wm->my + i, 0x000000FF);
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
