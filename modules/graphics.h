#ifndef GRAPHICS_1
#define GRAPHICS_1

#define COL(r, g, b, a) (r << 24 | g << 16 | b << 8 | a)

void graphics_fill(uint32* px, int px_width, int px_height, uint32 col);
void graphics_fill_rect(uint32* px, int px_width, int px_height, int x, int y, int w, int h, uint32 col);

#endif
#ifdef IMPL_GRAPHICS_1
#undef IMPL_GRAPHICS_1

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

#endif
