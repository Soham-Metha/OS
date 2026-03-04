#ifndef GRAPHICS_1
#define GRAPHICS_1
#include <common/types.h>

typedef struct gfx_canvas {
    uint32* px;
    int px_w;
    int px_h;
} GFX_Canvas;

#define GFX_CANVAS(p, w, h) (GFX_Canvas) { .px = p, .px_w = w, .px_h = h }

#define swap_points(ax, ay, bx, by) \
    do {                            \
        int tx = ax;                \
        ax     = bx;                \
        bx     = tx;                \
        int ty = ay;                \
        ay     = by;                \
        by     = ty;                \
    } while (0);

#define lerp(u, v, t) ((u) + ((v) - (u)) * ((float)t))
#define COL(r, g, b, a) (r << 24 | g << 16 | b << 8 | a)

uint32 gfx_lerp_color(uint32 a, uint32 b);
void gfx_fill(GFX_Canvas canvas, uint32 col);
bool gfx_put_pixel(GFX_Canvas canvas, int x, int y, uint32 col);
void gfx_fill_circ(GFX_Canvas canvas, int x, int y, int r, uint32 col);
void gfx_fill_rect(GFX_Canvas canvas, int x, int y, int w, int h, uint32 col);
void gfx_draw_line(GFX_Canvas canvas, int x1, int y1, int x2, int y2, uint32 col);
void gfx_fill_rowspan(GFX_Canvas canvas, int y, int x1, int x2, uint32 col);
void gfx_pattern_checker(GFX_Canvas canvas, int x, int y, int w, int h, int box_side, uint32 fg, uint32 bg);
void gfx_pattern_circles(GFX_Canvas canvas, int x, int y, int w, int h, int row_cnt, int col_cnt, uint32 col);
void gfx_pattern_shapes(GFX_Canvas canvas, int x, int y, int w, int h, uint32 col);

#endif
#ifndef IMPL_GRAPHICS_1
#undef IMPL_GRAPHICS_1

static inline uint8 lerpc(int16 c1, int16 c2, int16 a)
{
    return c1 + (c2 - c1) * a / 255;
}

uint32 gfx_lerp_color(uint32 a, uint32 b)
{
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

    Color c1 = (Color) { .as_u32 = a };
    Color c2 = (Color) { .as_u32 = b };
    if (c2.as_[COL_A] == 0xFF)
        return c2.as_u32;

    c1.as_[COL_R] = lerpc(c1.as_[COL_R], c2.as_[COL_R], c2.as_[COL_A]);
    c1.as_[COL_G] = lerpc(c1.as_[COL_G], c2.as_[COL_G], c2.as_[COL_A]);
    c1.as_[COL_B] = lerpc(c1.as_[COL_B], c2.as_[COL_B], c2.as_[COL_A]);
    // c1.as_[COL_A] = lerpc(c1.as_[COL_A], c2.as_[COL_A], c2.as_[COL_A]);

    return c1.as_u32;
}

bool gfx_put_pixel(GFX_Canvas canvas, int x, int y, uint32 col)
{
    if (x < 0 || y < 0 || x >= canvas.px_w || y >= canvas.px_h)
        return false;

    canvas.px[y * canvas.px_w + x] = gfx_lerp_color(canvas.px[y * canvas.px_w + x], col);
    return true;
}

void gfx_fill(GFX_Canvas canvas,
    uint32 col)
{
    for (int i = 0; i < canvas.px_w * canvas.px_h; i++)
        canvas.px[i] = gfx_lerp_color(canvas.px[i], col);
}

void gfx_fill_rect(GFX_Canvas canvas, int x, int y, int w, int h, uint32 col)
{
    if (x < 0) {
        x = 0;
    }

    if (y < 0) {
        y = 0;
    }

    if (x + w > canvas.px_w)
        w = canvas.px_w - x;
    if (y + h > canvas.px_h)
        h = canvas.px_h - y;

    if (w <= 0 || h <= 0)
        return;

    for (int yy = y; yy < y + h; yy++) {
        uint32* row = &canvas.px[yy * canvas.px_w + x];
        for (int xx = 0; xx < w; xx++)
            row[xx] = gfx_lerp_color(row[xx], col);
    }
}

void gfx_fill_rowspan(GFX_Canvas canvas, int y, int x1, int x2, uint32 col)
{
    if (y < 0 || y >= canvas.px_h)
        return;

    if (x1 < 0)
        x1 = 0;
    if (x2 >= canvas.px_w)
        x2 = canvas.px_w - 1;

    uint32* row = canvas.px + y * canvas.px_w;

    for (int x = x1; x <= x2; x++)
        row[x] = gfx_lerp_color(row[x], col);
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

void gfx_pattern_checker(GFX_Canvas canvas, int x, int y, int w, int h, int box_side, uint32 fg, uint32 bg)
{
    int row_cnt = h / box_side - 1;
    int col_cnt = w / box_side - 1;
    for (int yy = 0; yy <= row_cnt; yy++) {
        for (int xx = 0; xx <= col_cnt; xx++) {
            uint32 col = bg;
            if ((xx + yy) % 2 == 0) {
                col = fg;
            }
            gfx_fill_rect(canvas,
                x + xx * box_side, y + yy * box_side,
                box_side, box_side, col);
        }
    }
}

void gfx_pattern_circles(GFX_Canvas canvas, int x, int y, int w, int h, int row_cnt, int col_cnt, uint32 col)
{
    int cell_w = w / col_cnt;
    int cell_h = h / row_cnt;
    for (int yy = 0; yy <= row_cnt; yy++) {
        for (int xx = 0; xx <= col_cnt; xx++) {
            int r = (cell_w / 2 > cell_h / 2) ? cell_h / 2 : cell_w / 2;
            r     = lerp(r / 2, r, ((float)xx / col_cnt + (float)yy / row_cnt) / 2);
            gfx_fill_circ(canvas,
                x + xx * cell_w + r, y + yy * cell_h + r,
                r, col);
        }
    }
}

void gfx_pattern_shapes(GFX_Canvas canvas, int x, int y, int w, int h, uint32 col)
{
    gfx_fill_triangle(canvas,
        80, 80, 320, 80, 200, 420,
        COL(0xFF, 0x00, 0xFF, 0xFF));

    gfx_fill_triangle(canvas,
        100, 100, 250, 200, 120, 350,
        COL(0x00, 0x00, 0xFF, 0xAA));

    gfx_fill_triangle(canvas,
        120, 150, 300, 220, 180, 380,
        COL(0xFF, 0xFF, 0x00, 0x88));

    gfx_draw_line(canvas, x, y, x + w, y, col);
    gfx_draw_line(canvas, x + w, y, x + w, y + h, col);
    gfx_draw_line(canvas, x + w, y + h, x, y + h, col);
    gfx_draw_line(canvas, x, y + h, x, y, col);

    gfx_draw_line(canvas, x, y, x + w / 4, y + h, col);
    gfx_draw_line(canvas, x + w, y, x + 3 * w / 4, y + h, col);

    gfx_draw_line(canvas, x + w, y + h, x, y, col);
    gfx_draw_line(canvas, x, y + h, x + w, y, col);

    gfx_draw_line(canvas, x + w / 2, y + h / 3, x + w / 2, y + h / 3, col);
}

#endif
