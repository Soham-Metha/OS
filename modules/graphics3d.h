#ifndef GRAPHICS3D_1
#define GRAPHICS3D_1
#include "graphics.h"
#include "math.h"

void gfx_3d_draw_point(GFX_Canvas canvas, int x, int y, int z, int r, uint32 col);
void gfx_3d_draw_line(GFX_Canvas canvas, int x1, int y1, int z1, int x2, int y2, int z2, uint32 col);
void gfx_3d_draw_cube(GFX_Canvas canvas, int x1, int y1, int z1, int x2, int y2, int z2, uint32 col);

#endif

#ifdef IMPL_GRAPHICS3D_1
#undef IMPL_GRAPHICS3D_1
#define SCALE_CONST 1

void gfx_3d_draw_point(GFX_Canvas canvas, int x, int y, int z, int r, uint32 col)
{
    if (z <= 0)
        return;

    int sx = (x * SCALE_CONST) / z;
    int sy = (y * SCALE_CONST) / z;

    gfx_fill_circ(canvas, sx, sy, r, col);
}

void gfx_3d_draw_line(GFX_Canvas canvas, int x1, int y1, int z1, int x2, int y2, int z2, uint32 col)
{
    if (z1 <= 0 || z2 <= 0)
        return;

    int sx1 = (x1 * SCALE_CONST) / z1;
    int sy1 = (y1 * SCALE_CONST) / z1;

    int sx2 = (x2 * SCALE_CONST) / z2;
    int sy2 = (y2 * SCALE_CONST) / z2;

    gfx_draw_line(canvas, sx1, sy1, sx2, sy2, col);
}

void gfx_3d_draw_cube(GFX_Canvas canvas, int x1, int y1, int z1, int x2, int y2, int z2, uint32 col)
{
    // Bottom
    gfx_3d_draw_line(canvas, x1, y1, z1, x2, y1, z1, col);     // A-B
    gfx_3d_draw_line(canvas, x2, y1, z1, x2, y2, z1, col);     // B-C
    gfx_3d_draw_line(canvas, x2, y2, z1, x1, y2, z1, col);     // C-D
    gfx_3d_draw_line(canvas, x1, y2, z1, x1, y1, z1, col);     // D-A

    // Top
    gfx_3d_draw_line(canvas, x1, y1, z2, x2, y1, z2, col);     // E-F
    gfx_3d_draw_line(canvas, x2, y1, z2, x2, y2, z2, col);     // F-G
    gfx_3d_draw_line(canvas, x2, y2, z2, x1, y2, z2, col);     // G-H
    gfx_3d_draw_line(canvas, x1, y2, z2, x1, y1, z2, col);     // H-E

    // Vertical
    gfx_3d_draw_line(canvas, x1, y1, z1, x1, y1, z2, col);     // A-E
    gfx_3d_draw_line(canvas, x2, y1, z1, x2, y1, z2, col);     // B-F
    gfx_3d_draw_line(canvas, x2, y2, z1, x2, y2, z2, col);     // C-G
    gfx_3d_draw_line(canvas, x1, y2, z1, x1, y2, z2, col);     // D-H
}

void rotate_z(float* x, float* y, float angle)
{
    float s  = fast_sin(angle);
    float c  = fast_cos(angle);

    float nx = *x * c - *y * s;
    float ny = *x * s + *y * c;

    *x       = nx;
    *y       = ny;
}

void rotate_fast(float* x, float* y, float sin, float cos)
{
    float nx = *x * cos - *y * sin;
    float ny = *x * sin + *y * cos;

    *x       = nx;
    *y       = ny;
}

static float angle = 0;
// TODO: switch over to matrix for cleaner operations
void gfx_3d_test(GFX_Canvas canvas)
{
    angle += 0.02f;
    float s1        = fast_sin(angle * 0.7f);
    float c1        = fast_cos(angle * 0.7f);
    float s2        = fast_sin(angle);
    float c2        = fast_cos(angle);
    float s3        = fast_sin(angle * 0.5f);
    float c3        = fast_cos(angle * 0.5f);

    int grid_count  = 5;
    float grid_pad  = 0.5f / grid_count;
    float grid_size = ((grid_count - 1) * grid_pad);

    for (int cz = 0; cz < grid_count; cz++) {
        for (int cy = 0; cy < grid_count; cy++) {
            for (int cx = 0; cx < grid_count; cx++) {

                float x = cx * grid_pad - grid_size / 2;
                float y = cy * grid_pad - grid_size / 2;
                float z = cz * grid_pad - grid_size / 2;

                rotate_fast(&x, &y, s1, c1);     // GPT suggested anim values
                rotate_fast(&y, &z, s2, c2);
                rotate_fast(&x, &z, s3, c3);

                z += 0.6f;     // move cube away from camera

                float px = x / z;
                float py = y / z;

                // normalize to screen
                px       = (px + 1) / 2;
                py       = (py + 1) / 2;

                uint8 r = (uint8)(((float)cx / (grid_count - 1)) * 255);
                uint8 g = (uint8)(((float)cy / (grid_count - 1)) * 255);
                uint8 b = (uint8)(((float)cz / (grid_count - 1)) * 255);

                gfx_fill_circ(canvas,
                    px * canvas.px_w,
                    py * canvas.px_h,
                    8 / canvas.scale,
                    COL(r, g, b, 255));
            }
        }
    }
}

#endif
