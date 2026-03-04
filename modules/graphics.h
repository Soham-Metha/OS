#ifndef GRAPHICS_1
#define GRAPHICS_1
#include <common/types.h>

#define COL(r, g, b, a) (r << 24 | g << 16 | b << 8 | a)

void graphics_fill(uint32* px, int px_width, int px_height, uint32 col);
bool graphics_put_pixel(uint32* px, int px_width, int px_height, int x, int y, uint32 col);
void graphics_fill_rect(uint32* px, int px_width, int px_height, int x, int y, int w, int h, uint32 col);
void graphics_fill_circ(uint32* px, int px_width, int px_height, int x, int y, int r, uint32 col);
void graphics_fill_rowspan(uint32* px, int px_width, int px_height, int y, int x1, int x2, uint32 col);

void graphics_checker_pattern(uint32* px, int px_width, int px_height, int x, int y, int w, int h, int box_side, uint32 fg, uint32 bg);
void graphics_circle_pattern(uint32* px, int px_width, int px_height, int x, int y, int w, int h, int box_side, uint32 col);

#endif
#ifdef IMPL_GRAPHICS_1
#undef IMPL_GRAPHICS_1

bool graphics_put_pixel(uint32* px, int px_width, int px_height,
    int x, int y, uint32 col)
{
    if (x < 0 || y < 0 || x >= px_width || y >= px_height)
        return false;

    px[y * px_width + x] = col;
    return true;
}

void graphics_fill(uint32* px, int px_width, int px_height,
    uint32 col)
{
    for (int i = 0; i < px_width * px_height; i++)
        px[i] = col;
}

void graphics_fill_rect(uint32* px, int px_width, int px_height,
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
            row[xx] = col;
    }
}

void graphics_fill_rowspan(uint32* px, int px_width, int px_height,
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
        row[x] = col;
}

void graphics_fill_circ(uint32* px, int px_width, int px_height, int xc, int yc, int r, uint32 col)
{     // See: bresenham's algorithm for circle drawing
    int x = 0, y = r;
    int d = 3 - 2 * r;

    while (y >= x) {
        graphics_fill_rowspan(px, px_width, px_height, yc + y, xc - x, xc + x, col);
        graphics_fill_rowspan(px, px_width, px_height, yc - y, xc - x, xc + x, col);
        graphics_fill_rowspan(px, px_width, px_height, yc + x, xc - y, xc + y, col);
        graphics_fill_rowspan(px, px_width, px_height, yc - x, xc - y, xc + y, col);

        if (d > 0) {
            y -= 1;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
        x += 1;
    }
}

void graphics_checker_pattern(uint32* px, int px_width, int px_height,
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
            graphics_fill_rect(px, px_width, px_height,
                x + xx * box_side, y + yy * box_side,
                box_side, box_side, col);
        }
    }
}

void graphics_circle_pattern(uint32* px, int px_width, int px_height,
    int x, int y, int w, int h, int box_side, uint32 col)
{
    int row_cnt = h / box_side - 1;
    int col_cnt = w / box_side - 1;
    for (int yy = 0; yy <= row_cnt; yy++) {
        for (int xx = 0; xx <= col_cnt; xx++) {
            int r = box_side / 2;
            r     = r * 0.5f + (r - r * 0.5f) * (((float)xx / col_cnt + (float)yy / row_cnt) / 2);     // lerp, r = r/2 -> r
            graphics_fill_circ(px, px_width, px_height,
                x + xx * box_side + r, y + yy * box_side + r,
                r, col);
        }
    }
}

#endif
