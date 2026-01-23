/* wm.h */

#ifndef WM_1
#define WM_1

#include "compositor.h"
#include "terminal.h"
#include <common/types.h>

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

void wm_render(WindowManager* wm);

#endif
#ifdef IMPL_WM_1
#undef IMPL_WM_1

#define IMPL_COMPOSITOR_1
#include "compositor.h"
#include "tty.h"
#include <common/heap.h>
#include <hal/font.h>

void wm_init(WindowManager* wm, Compositor* c)
{
    wm->count        = 0;
    wm->focused      = -1;
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
        /* TODO: route to app */
        tty_push_key(wm->fallback_tty, key);
    } else {
        tty_push_key(wm->fallback_tty, key);
    }
}

void wm_render(WindowManager* wm)
{
    compositor_render(wm->compositor);
}

#endif
