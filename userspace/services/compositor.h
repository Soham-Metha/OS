/* compositor.h */

#ifndef COMPOSITOR_1
#define COMPOSITOR_1

#include <common/types.h>
#define COMPOSITOR_MAX_SURFACES 4

typedef struct Surface {
    int x;
    int y;
    int width;
    int height;
    uint32* pixels;
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

void compositor_render(Compositor* c);

void surface_put_pixel(Surface* s, int x, int y, uint32 color);
void surface_fill_rect(Surface* s, int x, int y, int w, int h, uint32 color);
void surface_clear(Surface* s, uint32 color);
void surface_blit(Surface* s, int src_x, int src_y, int dst_x, int dst_y, int w, int h);
#endif

#ifdef IMPL_COMPOSITOR_1
#undef IMPL_COMPOSITOR_1
#include <hal/hal.h>

private
void blit_surface(Surface* s)
{
    if (!s || !s->visible || !s->pixels)
        return;

    int x0 = s->x < 0 ? 0 : s->x;
    int y0 = s->y < 0 ? 0 : s->y;
    int x1 = s->x + s->width;
    int y1 = s->y + s->height;

    if (x1 > (int)hal_get_width())
        x1 = (int)hal_get_width();
    if (y1 > (int)hal_get_height())
        y1 = (int)hal_get_height();

    for (int y = y0; y < y1; y++) {
        for (int x = x0; x < x1; x++) {
            int sx       = x - s->x;
            int sy       = y - s->y;

            uint32 color = s->pixels[sy * s->width + sx];
            if ((color & 0xFF) == 0)
                continue;

            hal_put_pixel(x, y, color);
        }
    }
}

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

void compositor_render(Compositor* c)
{
    if (!c)
        return;

    bool any_dirty = false;

    for (int i = 0; i < c->count; i++) {
        Surface* s = c->surfaces[i];
        if (s && s->visible && s->dirty) {
            blit_surface(s);
            s->dirty  = false;
            any_dirty = true;
        }
    }

    if (any_dirty) {
        hal_present();
    }
}

void surface_put_pixel(Surface* s, int x, int y, uint32 color)
{
    if (!s || !s->pixels)
        return;

    if (x < 0 || y < 0 || x >= s->width || y >= s->height)
        return;

    s->pixels[y * s->width + x] = color;
    s->dirty                    = true;
}

void surface_clear(Surface* s, uint32 color)
{
    if (!s || !s->pixels)
        return;

    int total = s->width * s->height;
    for (int i = 0; i < total; i++)
        s->pixels[i] = color;

    s->dirty = true;
}

void surface_fill_rect(Surface* s, int x, int y, int w, int h, uint32 color)
{
    if (!s || !s->pixels)
        return;

    if (x < 0) {
        w += x;
        x = 0;
    }

    if (y < 0) {
        h += y;
        y = 0;
    }

    if (x + w > s->width)
        w = s->width - x;
    if (y + h > s->height)
        h = s->height - y;

    if (w <= 0 || h <= 0)
        return;

    for (int yy = y; yy < y + h; yy++) {
        uint32* row = &s->pixels[SURF_IDX(s, x, yy)];
        for (int xx = 0; xx < w; xx++)
            row[xx] = color;
    }

    s->dirty = true;
}

void surface_blit(Surface* s, int src_x, int src_y, int dst_x, int dst_y, int w, int h)
{
    if (!s || !s->pixels)
        return;

    if (src_x < 0 || src_y < 0 || dst_x < 0 || dst_y < 0)
        return;

    if (src_x + w > s->width)
        w = s->width - src_x;
    if (dst_x + w > s->width)
        w = s->width - dst_x;
    if (src_y + h > s->height)
        h = s->height - src_y;
    if (dst_y + h > s->height)
        h = s->height - dst_y;

    if (w <= 0 || h <= 0)
        return;

    int y_start, y_end, y_step;

    if (dst_y > src_y) {
        y_start = h - 1;
        y_end   = -1;
        y_step  = -1;
    } else {
        y_start = 0;
        y_end   = h;
        y_step  = 1;
    }

    for (int y = y_start; y != y_end; y += y_step) {
        uint32* src = &s->pixels[SURF_IDX(s, src_x, src_y + y)];
        uint32* dst = &s->pixels[SURF_IDX(s, dst_x, dst_y + y)];

        for (int x = 0; x < w; x++)
            dst[x] = src[x];
    }

    s->dirty = true;
}

#endif
