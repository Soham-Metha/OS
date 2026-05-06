#ifndef GRAPHICS_1
#define GRAPHICS_1
#include <common/types.h>

typedef struct gfx_canvas {
    uint32* px;
    int px_w;
    int px_h;
    int px_stride;
    float scale;
} GFX_Canvas;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} Point3f;

typedef struct {
    float u;
    float v;
    float w;
} Point2f;

typedef struct {
    Point3f vertex[3];
    Point2f texture[3];
    float shade;
} Tri3f;

typedef struct
{
    float m[4][4];
} Mat4f;

enum {
    COL_A,
    COL_B,
    COL_G,
    COL_R,
};

typedef union Color {
    uint32 as_u32;
    uint8 as_[4];
} Color;

#define GFX_CANVAS(p, w, h, s) gfx_init_canvas(p, w, h, s)
#define swap_points(ax, ay, bx, by) \
    do {                            \
        int tx = ax;                \
        ax     = bx;                \
        bx     = tx;                \
        int ty = ay;                \
        ay     = by;                \
        by     = ty;                \
    } while (0);

#define swap(t, a, b) \
    do {              \
        t tmp = a;    \
        a     = b;    \
        b     = tmp;  \
    } while (0);

#define lerp(u, v, t) ((u) + ((v) - (u)) * ((float)t))
#define COL(r, g, b, a) (r << 24 | g << 16 | b << 8 | a)

GFX_Canvas gfx_init_canvas(uint32* px, int px_w, int px_h, float scale);
GFX_Canvas gfx_init_subcanvas(GFX_Canvas canvas, int x, int y, int w, int h);

uint32 gfx_lerp_color(uint32 a, uint32 b, int mode);
bool gfx_put_pixel(GFX_Canvas canvas, int x, int y, uint32 col);

void gfx_fill(GFX_Canvas canvas, uint32 col);
void gfx_fill_circ(GFX_Canvas canvas, int x, int y, int r, uint32 col);
void gfx_fill_rect(GFX_Canvas canvas, int x, int y, int w, int h, uint32 col);
void gfx_fill_rowspan(GFX_Canvas canvas, int y, int x1, int x2, uint32 col);
void gfx_fill_triangle(GFX_Canvas canvas, int x0, int y0, int x1, int y1, int x2, int y2, uint32 col);

void gfx_draw_line(GFX_Canvas canvas, int x1, int y1, int x2, int y2, uint32 col);
void gfx_draw_triangle(GFX_Canvas canvas, int x0, int y0, int x1, int y1, int x2, int y2, uint32 col);

void gfx_fill_textured(GFX_Canvas dest, GFX_Canvas src);
void gfx_fill_textured_rowspan(GFX_Canvas dest, GFX_Canvas src, int y, int x1, int x2, float u1, float v1, float w1, float u2, float v2, float w2, float* depth_buff, float shade);
void gfx_fill_textured_triangle(GFX_Canvas dest, GFX_Canvas src, Point2f tex[3], int x0, int y0, int x1, int y1, int x2, int y2, float* depth_buff, float shade);

#endif
#ifdef IMPL_GRAPHICS_1
#undef IMPL_GRAPHICS_1

static inline uint8 gfx_blend_color(int16 c1, int16 c2, int16 a)
{
    return c1 + (c2 - c1) * a / 255;
}

bool gfx_blit_rect(int px_w, int px_h, int x, int y, int w, int h, int* x1, int* y1, int* x2, int* y2)
{
    *x1 = x;
    *y1 = y;
    *x2 = *x1 + w;
    *y2 = *y1 + h;

    if      (w > 0)  { *x2 -= 1; }
    else if (w < 0)  { *x2 += 1; }
    if  (*x2 < *x1)  swap(int,*x1, *x2);

    if      (h > 0)  { *y2 -= 1; }
    else if (h < 0)  { *y2 += 1; }
    if  (*y2 < *y1)  swap(int,*y1, *y2);

    if (*x1 >= px_w || *x2 < 0) return false;
    if (*y1 >= px_h || *y2 < 0) return false;

    if (*x1 < 0)     *x1 = 0;
    if (*y1 < 0)     *y1 = 0;
    if (*x2 >= px_w) *x2 = px_w - 1;
    if (*y2 >= px_h) *y2 = px_h - 1;

    return true;
}

GFX_Canvas gfx_init_canvas(uint32* px, int px_w, int px_h, float scale)
{
    GFX_Canvas res = (GFX_Canvas) {
        .px        = px,
        .px_w      = px_w / scale,
        .px_h      = px_h / scale,
        .px_stride = px_w,
        .scale     = scale,
    };

    return res;
}

GFX_Canvas gfx_init_subcanvas(GFX_Canvas canvas, int x, int y, int w, int h)
{
    GFX_Canvas res = { 0 };
    int x1, y1, x2, y2;
    if (gfx_blit_rect(canvas.px_w, canvas.px_h, x, y, w, h, &x1, &y1, &x2, &y2)) {
        int phys_x = (int)(x * canvas.scale);
        int phys_y = (int)(y * canvas.scale);

        res        = (GFX_Canvas) {
                   .px        = &canvas.px[phys_y * canvas.px_stride + phys_x],
                   .px_w      = x2 - x1 + 1,
                   .px_h      = y2 - y1 + 1,
                   .px_stride = canvas.px_stride,
                   .scale     = canvas.scale,
        };
    }

    return res;
}

uint32 gfx_lerp_color(uint32 a, uint32 b, int mode)
{
    Color c1 = (Color) { .as_u32 = a };
    Color c2 = (Color) { .as_u32 = b };
    if (c2.as_[COL_A] == 0xFF)
        return c2.as_u32;

    if (mode == 0) {
        c1.as_[COL_R] = gfx_blend_color(c1.as_[COL_R], c2.as_[COL_R], c2.as_[COL_A]);
        c1.as_[COL_G] = gfx_blend_color(c1.as_[COL_G], c2.as_[COL_G], c2.as_[COL_A]);
        c1.as_[COL_B] = gfx_blend_color(c1.as_[COL_B], c2.as_[COL_B], c2.as_[COL_A]);
        // c1.as_[COL_A] = gfx_blend_color(c1.as_[COL_A], c2.as_[COL_A], c2.as_[COL_A]);
    } else {
        c1.as_[COL_R] = gfx_blend_color(c1.as_[COL_R], c2.as_[COL_R], c2.as_[COL_A] / 4);
        c1.as_[COL_G] = gfx_blend_color(c1.as_[COL_G], c2.as_[COL_G], c2.as_[COL_A] / 4);
        c1.as_[COL_B] = gfx_blend_color(c1.as_[COL_B], c2.as_[COL_B], c2.as_[COL_A] / 4);
        // c1.as_[COL_A] = gfx_blend_color(c1.as_[COL_A], c2.as_[COL_A], c2.as_[COL_A] / 4);
    }

    return c1.as_u32;
}

static inline int round(float x)
{
    return (int)(x + (x >= 0 ? 0.5f : -0.5f));
}

bool gfx_put_pixel(GFX_Canvas canvas, int x, int y, uint32 col)
{
    if (x < 0 || y < 0 || x >= canvas.px_w || y >= canvas.px_h)
        return false;

    int phys_x = round(x * canvas.scale);
    int phys_y = round(y * canvas.scale);

    for (int dy = 0; dy < canvas.scale; dy++) {
        for (int dx = 0; dx < canvas.scale; dx++) {

            uint32* p = &canvas.px[(phys_y + dy) * canvas.px_stride + (phys_x + dx)];
            *p        = gfx_lerp_color(*p, col, canvas.scale < 1);
        }
    }

    return true;
}

void gfx_fill(GFX_Canvas canvas, uint32 col)
{
    for (int y = 0; y < canvas.px_h; y++) {
        for (int x = 0; x < canvas.px_w; x++) {
            gfx_put_pixel(canvas, x, y, col);
        }
    }
}

inline void gfx_fill_rect(GFX_Canvas canvas, int x, int y, int w, int h, uint32 col)
{
    gfx_fill(gfx_init_subcanvas(canvas, x, y, w, h), col);
}

inline void gfx_fill_rowspan(GFX_Canvas canvas, int y, int x1, int x2, uint32 col)
{
    if (x1 > x2) {
        int t = x1;
        x1    = x2;
        x2    = t;
    }

    gfx_fill(gfx_init_subcanvas(canvas, x1, y, x2 - x1 + 1, 1), col);
}

void gfx_draw_line(GFX_Canvas canvas, int x1, int y1, int x2, int y2, uint32 col)
{     // See: bresenham's algorithm for line drawing
    int dx = x2 - x1;
    int dy = y2 - y1;
    if (dx < 0)
        dx = -dx;
    if (dy > 0)
        dy = -dy;

    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;

    int er = dx + dy;

    while (1) {
        gfx_put_pixel(canvas, x1, y1, col);

        int e2 = 2 * er;

        if (e2 >= dy) {
            if (x1 == x2)
                break;
            er += dy;
            x1 += sx;
        }

        if (e2 <= dx) {
            if (y1 == y2)
                break;
            er += dx;
            y1 += sy;
        }
    }
}

void gfx_fill_triangle(GFX_Canvas canvas, int x0, int y0, int x1, int y1, int x2, int y2, uint32 col)
{
    if (y1 < y0)
        swap_points(x0, y0, x1, y1);
    if (y2 < y0)
        swap_points(x0, y0, x2, y2);
    if (y2 < y1)
        swap_points(x1, y1, x2, y2);

    for (int i = 0; i < (y2 - y0); i++) {
        if (i > y1 - y0 || y1 == y0) {
            gfx_fill_rowspan(canvas, y0 + i,
                lerp(x0, x2, i / (y2 - y0)),
                lerp(x1, x2, (i - (y1 - y0)) / (y2 - y1)),
                col);
        } else {
            gfx_fill_rowspan(canvas, y0 + i,
                lerp(x0, x2, i / (y2 - y0)),
                lerp(x0, x1, i / (y1 - y0)),
                col);
        }
    }
}

void gfx_draw_triangle(GFX_Canvas canvas, int x0, int y0, int x1, int y1, int x2, int y2, uint32 col)
{
    if (y1 < y0)
        swap_points(x0, y0, x1, y1);
    if (y2 < y0)
        swap_points(x0, y0, x2, y2);
    if (y2 < y1)
        swap_points(x1, y1, x2, y2);

    gfx_draw_line(canvas, x0, y0, x1, y1, col);
    gfx_draw_line(canvas, x1, y1, x2, y2, col);
    gfx_draw_line(canvas, x0, y0, x2, y2, col);
}

void gfx_fill_circ(GFX_Canvas canvas, int xc, int yc, int r, uint32 col)
{     // See: bresenham's algorithm for circle drawing
    int x = 0, y = r;
    int d = 3 - 2 * r;

    while (y >= x) {
        gfx_fill_rowspan(canvas, yc + y, xc - x, xc + x, col);
        gfx_fill_rowspan(canvas, yc - y, xc - x, xc + x, col);
        gfx_fill_rowspan(canvas, yc + x, xc - y, xc + y, col);
        gfx_fill_rowspan(canvas, yc - x, xc - y, xc + y, col);

        if (d > 0) {
            y -= 1;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
        x += 1;
    }
}

void gfx_fill_textured(GFX_Canvas dest, GFX_Canvas src)
{
    for (int y = 0; y < dest.px_h; y++) {
        int ny = y * src.px_h / dest.px_h;
        for (int x = 0; x < dest.px_w; x++) {
            int nx     = x * src.px_w / dest.px_w;
            uint32 col = src.px[ny * src.px_stride + nx];
            gfx_put_pixel(dest, x, y, col);
        }
    }
}

// TODO: improvements to data structs, add Point2i/Point3i, better swap functions etc
// TODO: textured circ & rect
// TODO: switch 'lerp's with 'step's
void gfx_fill_textured_triangle(
    GFX_Canvas dest, GFX_Canvas src,
    Point2f tex[3],
    int x0, int y0, int x1, int y1, int x2, int y2,
    float* depth_buff,
    float shade)
{
    if (y1 < y0) {
        swap_points(x0, y0, x1, y1);
        swap(Point2f, tex[0], tex[1]);
    }
    if (y2 < y0) {
        swap_points(x0, y0, x2, y2);
        swap(Point2f, tex[0], tex[2]);
    }
    if (y2 < y1) {
        swap_points(x1, y1, x2, y2);
        swap(Point2f, tex[1], tex[2]);
    }

    for (int i = 0; i < y2 - y0; i++) {
        float t1 = (float)i / (float)(y2 - y0);
        if ((i > y1 - y0) || (y1 == y0)) {
            float t2 = (float)(i - (y1 - y0)) / (float)(y2 - y1);

            gfx_fill_textured_rowspan(dest, src,
                y0 + i,
                lerp(x0, x2, t1),
                lerp(x1, x2, t2),
                lerp(tex[0].u, tex[2].u, t1),
                lerp(tex[0].v, tex[2].v, t1),
                lerp(tex[0].w, tex[2].w, t1),
                lerp(tex[1].u, tex[2].u, t2),
                lerp(tex[1].v, tex[2].v, t2),
                lerp(tex[1].w, tex[2].w, t2),
                depth_buff,
                shade);
        } else {
            float t2 = (float)i / (float)(y1 - y0);

            gfx_fill_textured_rowspan(dest, src,
                y0 + i,
                lerp(x0, x2, t1),
                lerp(x0, x1, t2),
                lerp(tex[0].u, tex[2].u, t1),
                lerp(tex[0].v, tex[2].v, t1),
                lerp(tex[0].w, tex[2].w, t1),
                lerp(tex[0].u, tex[1].u, t2),
                lerp(tex[0].v, tex[1].v, t2),
                lerp(tex[0].w, tex[1].w, t2),
                depth_buff,
                shade);
        }
    }
}

inline void gfx_fill_textured_rowspan(
    GFX_Canvas dest, GFX_Canvas src,
    int y,
    int x1, int x2,
    float u1, float v1, float w1,
    float u2, float v2, float w2,
    float* depth_buff,
    float shade)
{
    if (x1 > x2) {
        swap(int, x1, x2);
        swap(float, u1, u2);
        swap(float, v1, v2);
        swap(float, w1, w2);
    }

    if (x2 < 0 || x1 >= dest.px_w) return;

    if (x1 < 0)         x1 = 0;
    if (x2 > dest.px_w) x2 = dest.px_w;

    for (int x = 0; x < (x2 - x1); x++) {
        float t   = (float)(x) / (float)(x2 - x1);

        float w = lerp(w1, w2, t);

        int nx = (int)(lerp(u1, u2, t) / w * src.px_w);
        int ny = (int)(lerp(v1, v2, t) / w * src.px_h);
        int sx = x1 + x;
        int sy = y;

        Color col      = { .as_u32 = src.px[ny * src.px_stride + nx] };
        col.as_[COL_R] = shade * col.as_[COL_R];
        col.as_[COL_G] = shade * col.as_[COL_G];
        col.as_[COL_B] = shade * col.as_[COL_B];

        if (w > depth_buff[sy * dest.px_stride + sx]) {
            gfx_put_pixel(dest, sx, sy, col.as_u32);
            depth_buff[sy * dest.px_stride + sx] = w;
        }
    }
}

#endif
