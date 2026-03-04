#ifndef GRAPHICS_1
#define GRAPHICS_1
#include <common/types.h>

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
void gfx_fill(uint32* px, int px_width, int px_height, uint32 col);
bool gfx_put_pixel(uint32* px, int px_width, int px_height, int x, int y, uint32 col);
void gfx_fill_circ(uint32* px, int px_width, int px_height, int x, int y, int r, uint32 col);
void gfx_fill_rect(uint32* px, int px_width, int px_height, int x, int y, int w, int h, uint32 col);
void gfx_draw_line(uint32* px, int px_width, int px_height, int x1, int y1, int x2, int y2, uint32 col);
void gfx_fill_rowspan(uint32* px, int px_width, int px_height, int y, int x1, int x2, uint32 col);
void gfx_pattern_checker(uint32* px, int px_width, int px_height, int x, int y, int w, int h, int box_side, uint32 fg, uint32 bg);
void gfx_pattern_circles(uint32* px, int px_width, int px_height, int x, int y, int w, int h, int row_cnt, int col_cnt, uint32 col);
void gfx_pattern_shapes(uint32* px, int px_width, int px_height, int x, int y, int w, int h, uint32 col);

#endif
#ifdef IMPL_GRAPHICS_1
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
    c1.as_[COL_A] = lerpc(c1.as_[COL_A], c2.as_[COL_A], c2.as_[COL_A]);

    return c1.as_u32;
}

bool gfx_put_pixel(uint32* px, int px_width, int px_height,
    int x, int y, uint32 col)
{
    if (x < 0 || y < 0 || x >= px_width || y >= px_height)
        return false;

    px[y * px_width + x] = gfx_lerp_color(px[y * px_width + x], col);
    return true;
}

void gfx_fill(uint32* px, int px_width, int px_height,
    uint32 col)
{
    for (int i = 0; i < px_width * px_height; i++)
        px[i] = gfx_lerp_color(px[i], col);
}

void gfx_fill_rect(uint32* px, int px_width, int px_height,
    int x, int y, int w, int h, uint32 col)
{
    if (x < 0) {
        x = 0;
    }

    if (y < 0) {
        y = 0;
    }

    if (x + w > px_width)
        w = px_width - x;
    if (y + h > px_height)
        h = px_height - y;

    if (w <= 0 || h <= 0)
        return;

    for (int yy = y; yy < y + h; yy++) {
        uint32* row = &px[yy * px_width + x];
        for (int xx = 0; xx < w; xx++)
            row[xx] = gfx_lerp_color(row[xx], col);
    }
}

void gfx_fill_rowspan(uint32* px, int px_width, int px_height,
    int y, int x1, int x2, uint32 col)
{
    if (y < 0 || y >= px_height)
        return;

    if (x1 < 0)
        x1 = 0;
    if (x2 >= px_width)
        x2 = px_width - 1;

    uint32* row = px + y * px_width;

    for (int x = x1; x <= x2; x++)
        row[x] = gfx_lerp_color(row[x], col);
}

void gfx_draw_line(uint32* px, int px_width, int px_height,
    int x1, int y1, int x2, int y2, uint32 col)
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
        gfx_put_pixel(px, px_width, px_height, x1, y1, col);

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

void gfx_fill_triangle(uint32* px, int px_width, int px_height,
    int x0, int y0, int x1, int y1, int x2, int y2, uint32 col)
{
    if (y1 < y0)
        swap_points(x0, y0, x1, y1);
    if (y2 < y0)
        swap_points(x0, y0, x2, y2);
    if (y2 < y1)
        swap_points(x1, y1, x2, y2);

    for (int i = 0; i < (y2 - y0); i++) {
        if (i > y1 - y0 || y1 == y0) {
            gfx_fill_rowspan(px, px_width, px_height, y0 + i,
                lerp(x0, x2, i / (y2 - y0)),
                lerp(x1, x2, (i - (y1 - y0)) / (y2 - y1)),
                col);
        } else {
            gfx_fill_rowspan(px, px_width, px_height, y0 + i,
                lerp(x0, x2, i / (y2 - y0)),
                lerp(x0, x1, i / (y1 - y0)),
                col);
        }
    }
}

void gfx_fill_circ(uint32* px, int px_width, int px_height,
    int xc, int yc, int r, uint32 col)
{     // See: bresenham's algorithm for circle drawing
    int x = 0, y = r;
    int d = 3 - 2 * r;

    while (y >= x) {
        gfx_fill_rowspan(px, px_width, px_height, yc + y, xc - x, xc + x, col);
        gfx_fill_rowspan(px, px_width, px_height, yc - y, xc - x, xc + x, col);
        gfx_fill_rowspan(px, px_width, px_height, yc + x, xc - y, xc + y, col);
        gfx_fill_rowspan(px, px_width, px_height, yc - x, xc - y, xc + y, col);

        if (d > 0) {
            y -= 1;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
        x += 1;
    }
}

void gfx_pattern_checker(uint32* px, int px_width, int px_height,
    int x, int y, int w, int h, int box_side, uint32 fg, uint32 bg)
{
    int row_cnt = h / box_side - 1;
    int col_cnt = w / box_side - 1;
    for (int yy = 0; yy <= row_cnt; yy++) {
        for (int xx = 0; xx <= col_cnt; xx++) {
            uint32 col = bg;
            if ((xx + yy) % 2 == 0) {
                col = fg;
            }
            gfx_fill_rect(px, px_width, px_height,
                x + xx * box_side, y + yy * box_side,
                box_side, box_side, col);
        }
    }
}

void gfx_pattern_circles(uint32* px, int px_width, int px_height,
    int x, int y, int w, int h, int row_cnt, int col_cnt, uint32 col)
{
    int cell_w = w / col_cnt;
    int cell_h = h / row_cnt;
    for (int yy = 0; yy <= row_cnt; yy++) {
        for (int xx = 0; xx <= col_cnt; xx++) {
            int r = (cell_w / 2 > cell_h / 2) ? cell_h / 2 : cell_w / 2;
            r     = lerp(r / 2, r, ((float)xx / col_cnt + (float)yy / row_cnt) / 2);
            gfx_fill_circ(px, px_width, px_height,
                x + xx * cell_w + r, y + yy * cell_h + r,
                r, col);
        }
    }
}

void gfx_pattern_shapes(uint32* px, int px_width, int px_height,
    int x, int y, int w, int h, uint32 col)
{
    gfx_fill_triangle(px, px_width, px_height,
        80, 80, 320, 80, 200, 420,
        COL(0xFF, 0x00, 0xFF, 0xFF));

    gfx_fill_triangle(px, px_width, px_height,
        100, 100, 250, 200, 120, 350,
        COL(0x00, 0x00, 0xFF, 0xAA));

    gfx_fill_triangle(px, px_width, px_height,
        120, 150, 300, 220, 180, 380,
        COL(0xFF, 0xFF, 0x00, 0x88));

    gfx_draw_line(px, px_width, px_height, x, y, x + w, y, col);
    gfx_draw_line(px, px_width, px_height, x + w, y, x + w, y + h, col);
    gfx_draw_line(px, px_width, px_height, x + w, y + h, x, y + h, col);
    gfx_draw_line(px, px_width, px_height, x, y + h, x, y, col);

    gfx_draw_line(px, px_width, px_height, x, y, x + w / 4, y + h, col);
    gfx_draw_line(px, px_width, px_height, x + w, y, x + 3 * w / 4, y + h, col);

    gfx_draw_line(px, px_width, px_height, x + w, y + h, x, y, col);
    gfx_draw_line(px, px_width, px_height, x, y + h, x + w, y, col);

    gfx_draw_line(px, px_width, px_height, x + w / 2, y + h / 3, x + w / 2, y + h / 3, col);
}

#endif
