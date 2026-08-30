/*
 * compositor.h
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
#ifndef COMPOSITOR_1
#define COMPOSITOR_1

#include <common/types.h>
#include <common/gfx/graphics.h>
#define COMPOSITOR_MAX_SURFACES 4

typedef struct Surface {
    int x;
    int y;
    GFX_Canvas canvas;
    bool dirty;
    bool visible;
} Surface;

#define SURF_IDX(s, x, y) ((y) * (s)->width + (x))

typedef struct Compositor {
    Surface* surfaces[COMPOSITOR_MAX_SURFACES];
    int count;
    int width;
    int height;
} Compositor;

void compositor_init(Compositor* c, int width, int height);

bool compositor_attach(Compositor* c, struct Surface* s);
bool compositor_detach(Compositor* c, struct Surface* s);

bool compositor_raise(Compositor* c, struct Surface* s);
bool compositor_lower(Compositor* c, struct Surface* s);

void surface_put_pixel(Surface* s, int x, int y, uint32 color);
void surface_fill_rect(Surface* s, int x, int y, int w, int h, uint32 color);
void surface_clear(Surface* s, uint32 color);
void surface_blit(Surface* s, int src_x, int src_y, int dst_x, int dst_y, int w, int h);
#endif

#ifdef IMPL_COMPOSITOR_1
#undef IMPL_COMPOSITOR_1

void compositor_init(Compositor* c, int width, int height)
{
    if (!c)
        return;

    c->count  = 0;
    c->width  = width;
    c->height = height;
}

bool compositor_attach(Compositor* c, Surface* s)
{
    if (!c || !s)
        return false;
    if (c->count >= COMPOSITOR_MAX_SURFACES)
        return false;

    c->surfaces[c->count++] = s;
    s->dirty                = true;
    s->visible              = true;
    return true;
}

bool compositor_detach(Compositor* c, Surface* s)
{
    if (!c || !s)
        return false;

    for (int i = 0; i < c->count; i++) {
        if (c->surfaces[i] == s) {
            for (int j = i; j < c->count - 1; j++) {
                c->surfaces[j] = c->surfaces[j + 1];
            }
            c->surfaces[--c->count] = 0;
            return true;
        }
    }
    return false;
}

bool compositor_raise(Compositor* c, Surface* s)
{
    if (!c || !s)
        return false;

    int idx = -1;
    for (int i = 0; i < c->count; i++) {
        if (c->surfaces[i] == s) {
            idx = i;
            break;
        }
    }
    if (idx < 0)
        return false;

    for (int i = idx; i < c->count - 1; i++) {
        c->surfaces[i] = c->surfaces[i + 1];
    }
    c->surfaces[c->count - 1] = s;
    s->dirty                  = true;
    return true;
}

bool compositor_lower(Compositor* c, Surface* s)
{
    if (!c || !s)
        return false;

    int idx = -1;
    for (int i = 0; i < c->count; i++) {
        if (c->surfaces[i] == s) {
            idx = i;
            break;
        }
    }
    if (idx < 0)
        return false;

    for (int i = idx; i > 0; i--) {
        c->surfaces[i] = c->surfaces[i - 1];
    }
    c->surfaces[0] = s;
    s->dirty       = true;
    return true;
}

void surface_put_pixel(Surface* s, int x, int y, uint32 color)
{
    if (!s)
        return;

    if (gfx_put_pixel(s->canvas, x, y, color))
        s->dirty = true;
}

void surface_clear(Surface* s, uint32 color)
{
    if (!s)
        return;

    gfx_fill(s->canvas, color);
    s->dirty = true;
}

void surface_fill_rect(Surface* s, int x, int y, int w, int h, uint32 color)
{
    if (!s)
        return;

    gfx_fill_rect(s->canvas, x, y, w, h, color);
    s->dirty = true;
}

void surface_blit(Surface* s, int src_x, int src_y, int dst_x, int dst_y, int w, int h)
{
    if (!s)
        return;

    gfx_copy_rect(s->canvas, src_x, src_y, dst_x, dst_y, w, h);
    s->dirty = true;
}

#endif
