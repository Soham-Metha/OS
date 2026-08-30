/*
 * graphics.h
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
GFX_Canvas gfx_init_subcanvas(GFX_Canvas canvas, int x, int y, int w, int h, float scale);

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

#endif // GRAPHICS_1

#ifndef GRAPHICS3D_1
#define GRAPHICS3D_1

float p3f_len(Point3f a);
float p3f_dot(Point3f a, Point3f b);
Point3f p3f_cross(Point3f a, Point3f b);
Point3f p3f_normalize(Point3f a);
Point3f p3f_mul_mat(Point3f i, Mat4f m);
Point3f p3f_intersect_plane(Point3f plane_p, Point3f plane_n, Point3f line_start, Point3f line_end, float* t);
uint8 tri_clip(Point3f plane_p, Point3f plane_n, Tri3f in, Tri3f* out_tri1, Tri3f* out_tri2);

Point2f p2f_div(Point2f a, float b);
Point3f p3f_add(Point3f a, Point3f b);
Point3f p3f_sub(Point3f a, Point3f b);
Point3f p3f_mul(Point3f a, float b);
Point3f p3f_div(Point3f a, float b);

Mat4f tri_to_mat4(Tri3f t);
Tri3f mat4_to_tri(Mat4f m);

Mat4f matrix_mul(Mat4f A, Mat4f B);
Mat4f matrix_chain(int count, ...);
Mat4f matrix_project(float z_min, float z_max, float fov, int w, int h);
Mat4f matrix_viewport();

Mat4f matrix_trans(float x, float y, float z);
Mat4f matrix_scale(float x, float y, float z);
Mat4f matrix_rotX(float angle);
Mat4f matrix_rotY(float angle);
Mat4f matrix_rotZ(float angle);

#endif // GRAPHICS3D_1

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

static inline int round(float x)
{
    return (int)(x + (x >= 0 ? 0.5f : -0.5f));
}

GFX_Canvas gfx_init_subcanvas(GFX_Canvas canvas, int x, int y, int w, int h, float scale)
{
    GFX_Canvas res = { 0 };
    int x1, y1, x2, y2;

    if (gfx_blit_rect(canvas.px_w, canvas.px_h, x, y, w, h, &x1, &y1, &x2, &y2)) {
        int phys_x = (int)round(x * canvas.scale);
        int phys_y = (int)round(y * canvas.scale);

        int phys_w = x2 - x1 + 1;
        int phys_h = y2 - y1 + 1;

        scale = scale * canvas.scale;

        res = (GFX_Canvas) {
            .px        = &canvas.px[phys_y * canvas.px_stride + phys_x],
            .px_w      = (int)(phys_w / scale),
            .px_h      = (int)(phys_h / scale),
            .px_stride = canvas.px_stride,
            .scale     = scale,
        };
    }

    return res;
}

uint32 gfx_lerp_color(uint32 bg, uint32 fg, int mode)
{
    Color c1 = (Color) { .as_u32 = bg };
    Color c2 = (Color) { .as_u32 = fg };
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

bool gfx_put_pixel(GFX_Canvas canvas, int x, int y, uint32 col)
{
    if (x < 0 || y < 0 || x >= canvas.px_w || y >= canvas.px_h)
        return false;

    int phys_x = round(x * canvas.scale);
    int phys_y = round(y * canvas.scale);

    for (int dy = 0; dy < canvas.scale; dy++) {
        for (int dx = 0; dx < canvas.scale; dx++) {
        uint32* p = &canvas.px[(phys_y + dy) * canvas.px_stride + (phys_x + dx)];
           *p     = gfx_lerp_color(*p, col, canvas.scale < 1);
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
    gfx_fill(gfx_init_subcanvas(canvas, x, y, w, h, 1.0f), col);
}

inline void gfx_fill_rowspan(GFX_Canvas canvas, int y, int x1, int x2, uint32 col)
{
    if (x1 > x2) {
        int t = x1;
        x1    = x2;
        x2    = t;
    }

    gfx_fill(gfx_init_subcanvas(canvas, x1, y, x2 - x1 + 1, 1, 1.0f), col);
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

        if (w > depth_buff[sy * dest.px_w + sx]) { // depth buffer is malloced using width, not stride!
            gfx_put_pixel(dest, sx, sy, col.as_u32);
            depth_buff[sy * dest.px_w + sx] = w;
        }
    }
}

void gfx_copy_rect(GFX_Canvas canvas,
    int src_x, int src_y,
    int dst_x, int dst_y,
    int w, int h)
{
    if (src_x < 0 || src_y < 0 || dst_x < 0 || dst_y < 0)
        return;

    if (src_x + w > canvas.px_w)
        w = canvas.px_w - src_x;
    if (dst_x + w > canvas.px_w)
        w = canvas.px_w - dst_x;
    if (src_y + h > canvas.px_h)
        h = canvas.px_h - src_y;
    if (dst_y + h > canvas.px_h)
        h = canvas.px_h - dst_y;

    if (w <= 0 || h <= 0)
        return;

    int y_start, y_end, y_step;
    int x_start, x_end, x_step;

    if (dst_y > src_y) {
        y_start = h - 1;
        y_end   = -1;
        y_step  = -1;
    } else {
        y_start = 0;
        y_end   = h;
        y_step  = 1;
    }

    if (dst_x > src_x) {
        x_start = w - 1;
        x_end   = -1;
        x_step  = -1;
    } else {
        x_start = 0;
        x_end   = w;
        x_step  = 1;
    }

    for (int y = y_start; y != y_end; y += y_step) {
        uint32* src = canvas.px + (src_y + y) * canvas.px_stride;
        uint32* dst = canvas.px + (dst_y + y) * canvas.px_stride;

        for (int x = x_start; x != x_end; x += x_step)
            dst[dst_x + x] = src[src_x + x];
    }
}


#endif  // IMPL_GRAPHICS_1

#ifdef IMPL_GRAPHICS3D_1
#undef IMPL_GRAPHICS3D_1

#ifndef MATH_1
#define MATH_1
#define TABLE_SIZE 1024
#define PI 3.1415926f
#define TWO_PI 6.2831852f
#define INV_TWO_PI (TABLE_SIZE / TWO_PI)

static float sin_table[1024] = { 0.000000f, 0.006136f, 0.012272f, 0.018407f, 0.024541f, 0.030675f, 0.036807f, 0.042938f, 0.049068f, 0.055195f, 0.061321f, 0.067444f, 0.073565f, 0.079682f, 0.085797f, 0.091909f, 0.098017f, 0.104122f, 0.110222f, 0.116319f, 0.122411f, 0.128498f, 0.134581f, 0.140658f, 0.146730f, 0.152797f, 0.158858f, 0.164913f, 0.170962f, 0.177004f, 0.183040f, 0.189069f, 0.195090f, 0.201105f, 0.207111f, 0.213110f, 0.219101f, 0.225084f, 0.231058f, 0.237024f, 0.242980f, 0.248928f, 0.254866f, 0.260794f, 0.266713f, 0.272621f, 0.278520f, 0.284408f, 0.290285f, 0.296151f, 0.302006f, 0.307850f, 0.313682f, 0.319502f, 0.325310f, 0.331106f, 0.336890f, 0.342661f, 0.348419f, 0.354163f, 0.359895f, 0.365613f, 0.371317f, 0.377007f, 0.382683f, 0.388345f, 0.393992f, 0.399624f, 0.405241f, 0.410843f, 0.416430f, 0.422000f, 0.427555f, 0.433094f, 0.438616f, 0.444122f, 0.449611f, 0.455084f, 0.460539f, 0.465976f, 0.471397f, 0.476799f, 0.482184f, 0.487550f, 0.492898f, 0.498228f, 0.503538f, 0.508830f, 0.514103f, 0.519356f, 0.524590f, 0.529804f, 0.534998f, 0.540171f, 0.545325f, 0.550458f, 0.555570f, 0.560662f, 0.565732f, 0.570781f, 0.575808f, 0.580814f, 0.585798f, 0.590760f, 0.595699f, 0.600616f, 0.605511f, 0.610383f, 0.615232f, 0.620057f, 0.624859f, 0.629638f, 0.634393f, 0.639124f, 0.643831f, 0.648514f, 0.653173f, 0.657807f, 0.662416f, 0.667000f, 0.671559f, 0.676093f, 0.680601f, 0.685084f, 0.689541f, 0.693971f, 0.698376f, 0.702755f, 0.707107f, 0.711432f, 0.715731f, 0.720002f, 0.724247f, 0.728464f, 0.732654f, 0.736817f, 0.740951f, 0.745058f, 0.749136f, 0.753187f, 0.757209f, 0.761202f, 0.765167f, 0.769103f, 0.773010f, 0.776888f, 0.780737f, 0.784557f, 0.788346f, 0.792107f, 0.795837f, 0.799537f, 0.803208f, 0.806848f, 0.810457f, 0.814036f, 0.817585f, 0.821102f, 0.824589f, 0.828045f, 0.831470f, 0.834863f, 0.838225f, 0.841555f, 0.844854f, 0.848120f, 0.851355f, 0.854558f, 0.857729f, 0.860867f, 0.863973f, 0.867046f, 0.870087f, 0.873095f, 0.876070f, 0.879012f, 0.881921f, 0.884797f, 0.887640f, 0.890449f, 0.893224f, 0.895966f, 0.898674f, 0.901349f, 0.903989f, 0.906596f, 0.909168f, 0.911706f, 0.914210f, 0.916679f, 0.919114f, 0.921514f, 0.923880f, 0.926210f, 0.928506f, 0.930767f, 0.932993f, 0.935184f, 0.937339f, 0.939459f, 0.941544f, 0.943593f, 0.945607f, 0.947586f, 0.949528f, 0.951435f, 0.953306f, 0.955141f, 0.956940f, 0.958703f, 0.960431f, 0.962121f, 0.963776f, 0.965394f, 0.966976f, 0.968522f, 0.970031f, 0.971504f, 0.972940f, 0.974339f, 0.975702f, 0.977028f, 0.978317f, 0.979570f, 0.980785f, 0.981964f, 0.983105f, 0.984210f, 0.985278f, 0.986308f, 0.987301f, 0.988258f, 0.989177f, 0.990058f, 0.990903f, 0.991710f, 0.992480f, 0.993212f, 0.993907f, 0.994565f, 0.995185f, 0.995767f, 0.996313f, 0.996820f, 0.997290f, 0.997723f, 0.998118f, 0.998476f, 0.998795f, 0.999078f, 0.999322f, 0.999529f, 0.999699f, 0.999831f, 0.999925f, 0.999981f, 1.000000f, 0.999981f, 0.999925f, 0.999831f, 0.999699f, 0.999529f, 0.999322f, 0.999078f, 0.998795f, 0.998476f, 0.998118f, 0.997723f, 0.997290f, 0.996820f, 0.996313f, 0.995767f, 0.995185f, 0.994565f, 0.993907f, 0.993212f, 0.992480f, 0.991710f, 0.990903f, 0.990058f, 0.989177f, 0.988258f, 0.987301f, 0.986308f, 0.985278f, 0.984210f, 0.983105f, 0.981964f, 0.980785f, 0.979570f, 0.978317f, 0.977028f, 0.975702f, 0.974339f, 0.972940f, 0.971504f, 0.970031f, 0.968522f, 0.966976f, 0.965394f, 0.963776f, 0.962121f, 0.960431f, 0.958704f, 0.956940f, 0.955141f, 0.953306f, 0.951435f, 0.949528f, 0.947586f, 0.945607f, 0.943594f, 0.941544f, 0.939459f, 0.937339f, 0.935184f, 0.932993f, 0.930767f, 0.928506f, 0.926210f, 0.923880f, 0.921514f, 0.919114f, 0.916679f, 0.914210f, 0.911706f, 0.909168f, 0.906596f, 0.903989f, 0.901349f, 0.898674f, 0.895966f, 0.893224f, 0.890449f, 0.887640f, 0.884797f, 0.881921f, 0.879012f, 0.876070f, 0.873095f, 0.870087f, 0.867046f, 0.863973f, 0.860867f, 0.857729f, 0.854558f, 0.851355f, 0.848120f, 0.844854f, 0.841555f, 0.838225f, 0.834863f, 0.831470f, 0.828045f, 0.824589f, 0.821102f, 0.817585f, 0.814036f, 0.810457f, 0.806848f, 0.803208f, 0.799537f, 0.795837f, 0.792107f, 0.788346f, 0.784557f, 0.780737f, 0.776888f, 0.773010f, 0.769103f, 0.765167f, 0.761203f, 0.757209f, 0.753187f, 0.749137f, 0.745058f, 0.740951f, 0.736817f, 0.732654f, 0.728464f, 0.724247f, 0.720003f, 0.715731f, 0.711432f, 0.707107f, 0.702755f, 0.698376f, 0.693972f, 0.689541f, 0.685084f, 0.680601f, 0.676093f, 0.671559f, 0.667000f, 0.662416f, 0.657807f, 0.653173f, 0.648514f, 0.643832f, 0.639125f, 0.634393f, 0.629638f, 0.624860f, 0.620057f, 0.615232f, 0.610383f, 0.605511f, 0.600617f, 0.595699f, 0.590760f, 0.585798f, 0.580814f, 0.575808f, 0.570781f, 0.565732f, 0.560662f, 0.555570f, 0.550458f, 0.545325f, 0.540172f, 0.534998f, 0.529804f, 0.524590f, 0.519356f, 0.514103f, 0.508830f, 0.503539f, 0.498228f, 0.492898f, 0.487550f, 0.482184f, 0.476799f, 0.471397f, 0.465977f, 0.460539f, 0.455084f, 0.449611f, 0.444122f, 0.438616f, 0.433094f, 0.427555f, 0.422000f, 0.416430f, 0.410843f, 0.405241f, 0.399624f, 0.393992f, 0.388345f, 0.382683f, 0.377007f, 0.371317f, 0.365613f, 0.359895f, 0.354164f, 0.348419f, 0.342661f, 0.336890f, 0.331106f, 0.325310f, 0.319502f, 0.313682f, 0.307850f, 0.302006f, 0.296151f, 0.290285f, 0.284408f, 0.278520f, 0.272622f, 0.266713f, 0.260794f, 0.254866f, 0.248928f, 0.242980f, 0.237024f, 0.231058f, 0.225084f, 0.219101f, 0.213110f, 0.207111f, 0.201105f, 0.195091f, 0.189069f, 0.183040f, 0.177004f, 0.170962f, 0.164913f, 0.158858f, 0.152797f, 0.146731f, 0.140658f, 0.134581f, 0.128498f, 0.122411f, 0.116319f, 0.110222f, 0.104122f, 0.098017f, 0.091909f, 0.085797f, 0.079683f, 0.073565f, 0.067444f, 0.061321f, 0.055195f, 0.049068f, 0.042938f, 0.036807f, 0.030675f, 0.024541f, 0.018407f, 0.012272f, 0.006136f, 0.000000f, -0.006136f, -0.012271f, -0.018407f, -0.024541f, -0.030675f, -0.036807f, -0.042938f, -0.049067f, -0.055195f, -0.061321f, -0.067444f, -0.073564f, -0.079682f, -0.085797f, -0.091909f, -0.098017f, -0.104122f, -0.110222f, -0.116319f, -0.122411f, -0.128498f, -0.134580f, -0.140658f, -0.146730f, -0.152797f, -0.158858f, -0.164913f, -0.170962f, -0.177004f, -0.183040f, -0.189069f, -0.195090f, -0.201105f, -0.207111f, -0.213110f, -0.219101f, -0.225084f, -0.231058f, -0.237023f, -0.242980f, -0.248927f, -0.254866f, -0.260794f, -0.266713f, -0.272621f, -0.278520f, -0.284407f, -0.290284f, -0.296151f, -0.302006f, -0.307849f, -0.313682f, -0.319502f, -0.325310f, -0.331106f, -0.336890f, -0.342661f, -0.348419f, -0.354163f, -0.359895f, -0.365613f, -0.371317f, -0.377007f, -0.382683f, -0.388345f, -0.393992f, -0.399624f, -0.405241f, -0.410843f, -0.416429f, -0.422000f, -0.427555f, -0.433094f, -0.438616f, -0.444122f, -0.449611f, -0.455083f, -0.460539f, -0.465976f, -0.471397f, -0.476799f, -0.482184f, -0.487550f, -0.492898f, -0.498228f, -0.503538f, -0.508830f, -0.514103f, -0.519356f, -0.524589f, -0.529803f, -0.534997f, -0.540171f, -0.545325f, -0.550458f, -0.555570f, -0.560661f, -0.565732f, -0.570781f, -0.575808f, -0.580814f, -0.585798f, -0.590760f, -0.595699f, -0.600616f, -0.605511f, -0.610383f, -0.615231f, -0.620057f, -0.624859f, -0.629638f, -0.634393f, -0.639124f, -0.643831f, -0.648514f, -0.653173f, -0.657807f, -0.662416f, -0.667000f, -0.671559f, -0.676093f, -0.680601f, -0.685084f, -0.689540f, -0.693971f, -0.698376f, -0.702755f, -0.707107f, -0.711432f, -0.715731f, -0.720002f, -0.724247f, -0.728464f, -0.732654f, -0.736816f, -0.740951f, -0.745058f, -0.749136f, -0.753187f, -0.757209f, -0.761202f, -0.765167f, -0.769103f, -0.773010f, -0.776888f, -0.780737f, -0.784557f, -0.788346f, -0.792106f, -0.795837f, -0.799537f, -0.803207f, -0.806847f, -0.810457f, -0.814036f, -0.817585f, -0.821102f, -0.824589f, -0.828045f, -0.831469f, -0.834863f, -0.838225f, -0.841555f, -0.844853f, -0.848120f, -0.851355f, -0.854558f, -0.857729f, -0.860867f, -0.863973f, -0.867046f, -0.870087f, -0.873095f, -0.876070f, -0.879012f, -0.881921f, -0.884797f, -0.887639f, -0.890449f, -0.893224f, -0.895966f, -0.898674f, -0.901349f, -0.903989f, -0.906596f, -0.909168f, -0.911706f, -0.914210f, -0.916679f, -0.919114f, -0.921514f, -0.923880f, -0.926210f, -0.928506f, -0.930767f, -0.932993f, -0.935183f, -0.937339f, -0.939459f, -0.941544f, -0.943593f, -0.945607f, -0.947585f, -0.949528f, -0.951435f, -0.953306f, -0.955141f, -0.956940f, -0.958703f, -0.960430f, -0.962121f, -0.963776f, -0.965394f, -0.966976f, -0.968522f, -0.970031f, -0.971504f, -0.972940f, -0.974339f, -0.975702f, -0.977028f, -0.978317f, -0.979570f, -0.980785f, -0.981964f, -0.983105f, -0.984210f, -0.985278f, -0.986308f, -0.987301f, -0.988258f, -0.989176f, -0.990058f, -0.990903f, -0.991710f, -0.992480f, -0.993212f, -0.993907f, -0.994565f, -0.995185f, -0.995767f, -0.996313f, -0.996820f, -0.997290f, -0.997723f, -0.998118f, -0.998476f, -0.998795f, -0.999078f, -0.999322f, -0.999529f, -0.999699f, -0.999831f, -0.999925f, -0.999981f, -1.000000f, -0.999981f, -0.999925f, -0.999831f, -0.999699f, -0.999529f, -0.999322f, -0.999078f, -0.998795f, -0.998476f, -0.998118f, -0.997723f, -0.997290f, -0.996820f, -0.996313f, -0.995767f, -0.995185f, -0.994565f, -0.993907f, -0.993212f, -0.992480f, -0.991710f, -0.990903f, -0.990058f, -0.989177f, -0.988258f, -0.987301f, -0.986308f, -0.985278f, -0.984210f, -0.983106f, -0.981964f, -0.980785f, -0.979570f, -0.978317f, -0.977028f, -0.975702f, -0.974339f, -0.972940f, -0.971504f, -0.970031f, -0.968522f, -0.966977f, -0.965394f, -0.963776f, -0.962121f, -0.960431f, -0.958704f, -0.956940f, -0.955141f, -0.953306f, -0.951435f, -0.949528f, -0.947586f, -0.945607f, -0.943594f, -0.941544f, -0.939459f, -0.937339f, -0.935184f, -0.932993f, -0.930767f, -0.928506f, -0.926210f, -0.923880f, -0.921514f, -0.919114f, -0.916679f, -0.914210f, -0.911706f, -0.909168f, -0.906596f, -0.903989f, -0.901349f, -0.898675f, -0.895966f, -0.893224f, -0.890449f, -0.887640f, -0.884797f, -0.881921f, -0.879012f, -0.876070f, -0.873095f, -0.870087f, -0.867046f, -0.863973f, -0.860867f, -0.857729f, -0.854558f, -0.851355f, -0.848121f, -0.844854f, -0.841555f, -0.838225f, -0.834863f, -0.831470f, -0.828045f, -0.824589f, -0.821103f, -0.817585f, -0.814036f, -0.810457f, -0.806848f, -0.803208f, -0.799537f, -0.795837f, -0.792107f, -0.788347f, -0.784557f, -0.780738f, -0.776889f, -0.773011f, -0.769104f, -0.765168f, -0.761203f, -0.757209f, -0.753187f, -0.749137f, -0.745058f, -0.740951f, -0.736817f, -0.732654f, -0.728465f, -0.724247f, -0.720003f, -0.715731f, -0.711432f, -0.707107f, -0.702755f, -0.698376f, -0.693972f, -0.689541f, -0.685084f, -0.680601f, -0.676093f, -0.671559f, -0.667000f, -0.662416f, -0.657807f, -0.653173f, -0.648515f, -0.643832f, -0.639125f, -0.634394f, -0.629638f, -0.624860f, -0.620057f, -0.615232f, -0.610383f, -0.605511f, -0.600617f, -0.595699f, -0.590760f, -0.585798f, -0.580814f, -0.575808f, -0.570781f, -0.565732f, -0.560662f, -0.555570f, -0.550458f, -0.545325f, -0.540172f, -0.534998f, -0.529804f, -0.524590f, -0.519356f, -0.514103f, -0.508830f, -0.503539f, -0.498228f, -0.492898f, -0.487550f, -0.482184f, -0.476799f, -0.471397f, -0.465977f, -0.460539f, -0.455084f, -0.449612f, -0.444122f, -0.438616f, -0.433094f, -0.427555f, -0.422000f, -0.416430f, -0.410843f, -0.405241f, -0.399625f, -0.393992f, -0.388345f, -0.382684f, -0.377008f, -0.371318f, -0.365613f, -0.359895f, -0.354164f, -0.348419f, -0.342661f, -0.336890f, -0.331107f, -0.325311f, -0.319502f, -0.313682f, -0.307850f, -0.302006f, -0.296151f, -0.290285f, -0.284408f, -0.278520f, -0.272621f, -0.266713f, -0.260794f, -0.254866f, -0.248928f, -0.242981f, -0.237024f, -0.231059f, -0.225084f, -0.219102f, -0.213111f, -0.207112f, -0.201105f, -0.195091f, -0.189069f, -0.183040f, -0.177005f, -0.170962f, -0.164913f, -0.158858f, -0.152797f, -0.146731f, -0.140658f, -0.134581f, -0.128498f, -0.122411f, -0.116319f, -0.110222f, -0.104122f, -0.098017f, -0.091909f, -0.085797f, -0.079683f, -0.073565f, -0.067444f, -0.061321f, -0.055196f, -0.049068f, -0.042939f, -0.036808f, -0.030675f, -0.024542f, -0.018407f, -0.012272f, -0.006136f,
};

float fast_sin(float angle)
{
    int idx = (int)(angle * INV_TWO_PI);

    idx &= (TABLE_SIZE - 1);

    return sin_table[idx];
}

float fast_cos(float angle)
{
    int idx = (int)(angle * INV_TWO_PI);

    idx = (idx + TABLE_SIZE / 4) & (TABLE_SIZE - 1);

    return sin_table[idx];
}

float fast_tan(float angle)
{
    return fast_sin(angle)/fast_cos(angle);
}

float Q_rsqrt(float number)
{
    long i;
    float x2, y;
    const float threehalfs = 1.5F;

    union {
        float f;
        long l;
    } u;

    x2 = number * 0.5F;
    y  = number;
    u.f = y;
    i = u.l;
    i  = 0x5f3759df - (i >> 1);
    u.l = i;
    y  = u.f;
    y  = y * (threehalfs - (x2 * y * y));

    return y;
}

float fast_sqrt(float x) {
    return x * Q_rsqrt(x);
}

#endif

#define SCALE_CONST 1

float p3f_dot(Point3f a, Point3f b)
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

float p3f_len(Point3f a)
{
    return fast_sqrt(p3f_dot(a, a));
}

Point2f p2f_div(Point2f a, float b)
{
    return (Point2f) {
        .u = a.u / b,
        .v = a.v / b,
    };
}

Point3f p3f_add(Point3f a, Point3f b)
{
    return (Point3f) {
        .x = a.x + b.x,
        .y = a.y + b.y,
        .z = a.z + b.z,
    };
}

Point3f p3f_sub(Point3f a, Point3f b)
{
    return (Point3f) {
        .x = a.x - b.x,
        .y = a.y - b.y,
        .z = a.z - b.z,
    };
}

Point3f p3f_mul(Point3f a, float b)
{
    return (Point3f) {
        .x = a.x * b,
        .y = a.y * b,
        .z = a.z * b,
    };
}

Point3f p3f_div(Point3f a, float b)
{
    return (Point3f) {
        .x = a.x / b,
        .y = a.y / b,
        .z = a.z / b,
    };
}

Point3f p3f_cross(Point3f a, Point3f b)
{
    return (Point3f) {
        .x = a.y * b.z - a.z * b.y,
        .y = a.z * b.x - a.x * b.z,
        .z = a.x * b.y - a.y * b.x,
    };
}

Point3f p3f_normalize(Point3f a)
{
    float len = p3f_len(a);
    return (Point3f) {
        .x = a.x / len,
        .y = a.y / len,
        .z = a.z / len,
    };
}

Point3f p3f_mul_mat(Point3f i, Mat4f m)
{
    return (Point3f) {
        .x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0],
        .y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1],
        .z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2],
        .w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3],
    };
}

Point3f p3f_intersect_plane(Point3f plane_p, Point3f plane_n, Point3f line_start, Point3f line_end, float* t)
{
    plane_n       = p3f_normalize(plane_n);
    float d_plane = p3f_dot(plane_n, plane_p);
    float d_a     = p3f_dot(line_start, plane_n);
    float d_b     = p3f_dot(line_end, plane_n);

    *t            = (d_plane - d_a) / (d_b - d_a);
    // simple lerp
    Point3f line  = p3f_sub(line_end, line_start);     // B-A
    line          = p3f_mul(line, *t);                 // (B-A)*t
    return p3f_add(line_start, line);                  // A + (B-A)*t
}

uint8 tri_clip(Point3f plane_p, Point3f plane_n, Tri3f in, Tri3f* out_tri1, Tri3f* out_tri2)
{
    plane_n       = p3f_normalize(plane_n);
    float plane_d = p3f_dot(plane_n, plane_p);

#define DIST(p) (p3f_dot(plane_n, (p)) - plane_d)
    Point3f* inside[3];
    Point3f* outside[3];
    Point2f* inside_tex[3];
    Point2f* outside_tex[3];
    uint8 ins_cnt = 0;
    uint8 out_cnt = 0;

    if (0 <= DIST(in.vertex[0])) {
        inside[ins_cnt]       = &in.vertex[0];
        inside_tex[ins_cnt++] = &in.texture[0];
    } else {
        outside[out_cnt]       = &in.vertex[0];
        outside_tex[out_cnt++] = &in.texture[0];
    }

    if (0 <= DIST(in.vertex[1])) {
        inside[ins_cnt]       = &in.vertex[1];
        inside_tex[ins_cnt++] = &in.texture[1];
    } else {
        outside[out_cnt]       = &in.vertex[1];
        outside_tex[out_cnt++] = &in.texture[1];
    }

    if (0 <= DIST(in.vertex[2])) {
        inside[ins_cnt]       = &in.vertex[2];
        inside_tex[ins_cnt++] = &in.texture[2];
    } else {
        outside[out_cnt]       = &in.vertex[2];
        outside_tex[out_cnt++] = &in.texture[2];
    }

    if (ins_cnt == 0) {
        return 0;
    }
    if (ins_cnt == 3) {
        *out_tri1 = in;
        return 1;
    }

    if (ins_cnt == 1 && out_cnt == 2) {
        float t1, t2;
        Point3f new_p_1 = p3f_intersect_plane(plane_p, plane_n, *inside[0], *outside[0], &t1);
        Point3f new_p_2 = p3f_intersect_plane(plane_p, plane_n, *inside[0], *outside[1], &t2);
        Point2f new_t_1 = (Point2f) {
            .u = lerp(inside_tex[0]->u, outside_tex[0]->u, t1),
            .v = lerp(inside_tex[0]->v, outside_tex[0]->v, t1),
            .w = lerp(inside_tex[0]->w, outside_tex[0]->w, t1),
        };
        Point2f new_t_2 = (Point2f) {
            .u = lerp(inside_tex[0]->u, outside_tex[1]->u, t2),
            .v = lerp(inside_tex[0]->v, outside_tex[1]->v, t2),
            .w = lerp(inside_tex[0]->w, outside_tex[1]->w, t2),
        };
        *out_tri1 = (Tri3f) {
            .shade        = in.shade,
            .vertex[0]  = *inside[0],
            .vertex[1]  = new_p_1,
            .vertex[2]  = new_p_2,
            .texture[0] = *inside_tex[0],
            .texture[1] = new_t_1,
            .texture[2] = new_t_2,
        };
        return 1;
    }

    if (ins_cnt == 2 && out_cnt == 1) {
        float t1, t2;
        Point3f new_p_1 = p3f_intersect_plane(plane_p, plane_n, *inside[0], *outside[0], &t1);
        Point3f new_p_2 = p3f_intersect_plane(plane_p, plane_n, *inside[1], *outside[0], &t2);
        Point2f new_t_1 = (Point2f) {
            .u = lerp(inside_tex[0]->u, outside_tex[0]->u, t1),
            .v = lerp(inside_tex[0]->v, outside_tex[0]->v, t1),
            .w = lerp(inside_tex[0]->w, outside_tex[0]->w, t1),
        };
        Point2f new_t_2 = (Point2f) {
            .u = lerp(inside_tex[1]->u, outside_tex[0]->u, t2),
            .v = lerp(inside_tex[1]->v, outside_tex[0]->v, t2),
            .w = lerp(inside_tex[1]->w, outside_tex[0]->w, t2),
        };
        *out_tri1       = (Tri3f) {
                  .shade       = in.shade,
                  .vertex[0] = *inside[0],
                  .vertex[1] = *inside[1],
                  .vertex[2] = new_p_1,
                  .texture[0] = *inside_tex[0],
                  .texture[1] = *inside_tex[1],
                  .texture[2] = new_t_1,
        };
        *out_tri2 = (Tri3f) {
            .shade       = in.shade,
            .vertex[0] = *inside[1],
            .vertex[1] = new_p_1,
            .vertex[2] = new_p_2,
            .texture[0] = *inside_tex[1],
            .texture[1] = new_t_1,
            .texture[2] = new_t_2,
        };
        return 2;
    }
    return 0;
}

Mat4f tri_to_mat4(Tri3f t)
{
    return (Mat4f) {
        .m[0][0] = t.vertex[0].x,
        .m[0][1] = t.vertex[0].y,
        .m[0][2] = t.vertex[0].z,
        .m[0][3] = t.vertex[0].w,

        .m[1][0] = t.vertex[1].x,
        .m[1][1] = t.vertex[1].y,
        .m[1][2] = t.vertex[1].z,
        .m[1][3] = t.vertex[1].w,

        .m[2][0] = t.vertex[2].x,
        .m[2][1] = t.vertex[2].y,
        .m[2][2] = t.vertex[2].z,
        .m[2][3] = t.vertex[2].w,

        .m[3][0] = 0.0f,
        .m[3][1] = 0.0f,
        .m[3][2] = 0.0f,
        .m[3][3] = 1.0f,
    };
}

Tri3f mat4_to_tri(Mat4f m)
{
    Tri3f t;

    for (int i = 0; i < 3; i++) {
        t.vertex[i].x = m.m[i][0];
        t.vertex[i].y = m.m[i][1];
        t.vertex[i].z = m.m[i][2];
        t.vertex[i].w = m.m[i][3];
    }

    return t;
}

Mat4f matrix_mul(Mat4f A, Mat4f B)
{
    Mat4f R = { 0 };

    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            R.m[r][c] = A.m[r][0] * B.m[0][c]
                + A.m[r][1] * B.m[1][c]
                + A.m[r][2] * B.m[2][c]
                + A.m[r][3] * B.m[3][c];
        }
    }

    return R;
}

Mat4f matrix_chain(int count, ...)
{
    va_list args;
    va_start(args, count);

    Mat4f result = va_arg(args, Mat4f);     // first matrix

    for (int i = 1; i < count; i++) {
        Mat4f m = va_arg(args, Mat4f);
        result  = matrix_mul(result, m);
    }

    va_end(args);
    return result;
}

Mat4f matrix_project(float z_min, float z_max, float fov, int w, int h)
{
    float aspect_ratio = (float)h / w;
    float fov_rad = fov * (PI / 180.0f);
    float fov_tan = 1.0f / fast_tan(fov_rad * 0.5f);
    return (Mat4f) {
        .m[0][0] = aspect_ratio * fov_tan,
        .m[1][1] = fov_tan,
        .m[2][2] = z_max / (z_max - z_min),
        .m[3][2] = (-z_max * z_min) / (z_max - z_min),
        .m[2][3] = 1.0f,
        .m[3][3] = 0.0f,
    };
}

Mat4f matrix_viewport()
{
    return (Mat4f) {
        .m[0][0] = 0.5f,
        .m[1][1] = 0.5f,
        .m[2][2] = 1.0f,
        .m[3][3] = 1.0f,
        .m[3][0] = 0.5f,
        .m[3][1] = 0.5f,
    };
}

Mat4f matrix_trans(float x, float y, float z)
{
    return (Mat4f) {
        .m[0][0] = 1,
        .m[1][1] = 1,
        .m[2][2] = 1,
        .m[3][3] = 1,
        .m[3][0] = x,
        .m[3][1] = y,
        .m[3][2] = z,
    };
}

Mat4f matrix_scale(float x, float y, float z)
{
    return (Mat4f) {
        .m[0][0] = x,
        .m[1][1] = y,
        .m[2][2] = z,
        .m[3][3] = 1,
    };
}

Mat4f matrix_rotX(float angle)
{
    float c = fast_cos(angle);
    float s = fast_sin(angle);

    return (Mat4f) {
        .m[1][1] = c,
        .m[1][2] = s,
        .m[2][1] = -s,
        .m[2][2] = c,
        .m[3][3] = 1,
        .m[0][0] = 1,
    };
}

Mat4f matrix_rotY(float angle)
{
    float c = fast_cos(angle);
    float s = fast_sin(angle);

    return (Mat4f) {
        .m[0][0] = c,
        .m[0][2] = s,
        .m[1][1] = 1,
        .m[2][0] = -s,
        .m[2][2] = c,
        .m[3][3] = 1,
    };
}

Mat4f matrix_rotZ(float angle)
{
    float c = fast_cos(angle);
    float s = fast_sin(angle);

    return (Mat4f) {
        .m[0][0] = c,
        .m[0][1] = s,
        .m[1][0] = -s,
        .m[1][1] = c,
        .m[2][2] = 1,
        .m[3][3] = 1,
    };
}

Mat4f matrix_pointed(Point3f pos, Point3f tar, Point3f up)
{
    Point3f next_forward, next_up, next_right;
    next_forward = p3f_sub(tar, pos);
    next_forward = p3f_normalize(next_forward);

    Point3f b    = p3f_mul(next_forward, p3f_dot(next_forward, up));
    next_up      = p3f_sub(up, b);
    next_up      = p3f_normalize(next_up);

    next_right   = p3f_cross(next_up, next_forward);

    return (Mat4f) {
        .m[0][0] = next_right.x,     // vector pointing to camera 'x'
        .m[0][1] = next_right.y,
        .m[0][2] = next_right.z,

        .m[1][0] = next_up.x,     // vector pointing to camera 'y'
        .m[1][1] = next_up.y,
        .m[1][2] = next_up.z,

        .m[2][0] = next_forward.x,     // vector pointing to camera 'z'
        .m[2][1] = next_forward.y,
        .m[2][2] = next_forward.z,

        .m[3][0] = pos.x,     // translation as needed!
        .m[3][1] = pos.y,
        .m[3][2] = pos.z,
    };
}

// NOTE: only for translation/rotation
Mat4f matrix_inv(Mat4f a)
{
    return (Mat4f) {
        .m[0][0] = a.m[0][0],
        .m[0][1] = a.m[1][0],
        .m[0][2] = a.m[2][0],

        .m[1][0] = a.m[0][1],
        .m[1][1] = a.m[1][1],
        .m[1][2] = a.m[2][1],

        .m[2][0] = a.m[0][2],
        .m[2][1] = a.m[1][2],
        .m[2][2] = a.m[2][2],

        .m[3][0] = -(a.m[3][0] * a.m[0][0] + a.m[3][1] * a.m[0][1] + a.m[3][2] * a.m[0][2]),
        .m[3][1] = -(a.m[3][0] * a.m[1][0] + a.m[3][1] * a.m[1][1] + a.m[3][2] * a.m[1][2]),
        .m[3][2] = -(a.m[3][0] * a.m[2][0] + a.m[3][1] * a.m[2][1] + a.m[3][2] * a.m[2][2]),

        .m[3][3] = 1.0f,
    };
}

#endif  // IMPL_GRAPHICS3D_1
