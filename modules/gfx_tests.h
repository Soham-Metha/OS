#include "graphics3d.h"
#include <extras/fish.c>
#include <extras/gun.c>

Tri3f mesh[] = FISH_MESH;

void gfx_pattern_checker(GFX_Canvas canvas, int box_side, uint32 fg, uint32 bg)
{
    int row_cnt = canvas.px_h / box_side - 1;
    int col_cnt = canvas.px_w / box_side - 1;
    for (int yy = 0; yy <= row_cnt; yy++) {
        for (int xx = 0; xx <= col_cnt; xx++) {
            uint32 col = bg;
            if ((xx + yy) % 2 == 0) {
                col = fg;
            }
            gfx_fill_rect(canvas,
                xx * box_side, yy * box_side,
                box_side, box_side, col);
        }
    }
}

void gfx_pattern_circles(GFX_Canvas canvas, int row_cnt, int col_cnt, uint32 col)
{
    int cell_w = canvas.px_w / col_cnt;
    int cell_h = canvas.px_h / row_cnt;
    for (int yy = 0; yy <= row_cnt; yy++) {
        for (int xx = 0; xx <= col_cnt; xx++) {
            int r = (cell_w / 2 > cell_h / 2) ? cell_h / 2 : cell_w / 2;
            r     = lerp(r / 2, r, ((float)xx / col_cnt + (float)yy / row_cnt) / 2);
            gfx_fill_circ(canvas,
                xx * cell_w + r, yy * cell_h + r,
                r, col);
        }
    }
}

void gfx_pattern_shapes(GFX_Canvas canvas, uint32 col)
{
    gfx_fill_triangle(canvas,
        80 / canvas.scale, 80 / canvas.scale,
        320 / canvas.scale, 80 / canvas.scale,
        200 / canvas.scale, 420 / canvas.scale,
        COL(0xFF, 0x00, 0xFF, 0xFF));

    gfx_fill_triangle(canvas,
        100 / canvas.scale, 100 / canvas.scale,
        250 / canvas.scale, 200 / canvas.scale,
        120 / canvas.scale, 350 / canvas.scale,
        COL(0x00, 0x00, 0xFF, 0xAA));

    gfx_fill_triangle(canvas,
        120 / canvas.scale, 150 / canvas.scale,
        300 / canvas.scale, 220 / canvas.scale,
        180 / canvas.scale, 380 / canvas.scale,
        COL(0xFF, 0xFF, 0x00, 0x88));

    int xmin = 0;
    int ymin = 0;

    int xmax = canvas.px_w - 1;
    int ymax = canvas.px_h - 1;

    gfx_draw_line(canvas, xmin, ymin, xmax, ymin, col);
    gfx_draw_line(canvas, xmax, ymin, xmax, ymax, col);
    gfx_draw_line(canvas, xmax, ymax, xmin, ymax, col);
    gfx_draw_line(canvas, xmin, ymax, xmin, ymin, col);

    gfx_draw_line(canvas, xmin, ymin, xmax / 4, ymax, col);
    gfx_draw_line(canvas, xmax, ymin, 3 * xmax / 4, ymax, col);

    gfx_draw_line(canvas, xmax, ymax, xmin, ymin, col);
    gfx_draw_line(canvas, xmin, ymax, xmax, ymin, col);

    gfx_draw_line(canvas, (xmin + xmax) / 2, (ymin + ymax) / 3,
        (xmin + xmax) / 2, (ymin + ymax) / 3, col);
}

bool swap_cond(Tri3f t1, Tri3f t2)
{
    float z1 = (t1.vertex[0].z + t1.vertex[1].z + t1.vertex[2].z) / 3;
    float z2 = (t2.vertex[0].z + t2.vertex[1].z + t2.vertex[2].z) / 3;
    return z1 > z2;
}

void insert(Tri3f queue[], int idx, Tri3f tri)
{
    queue[idx] = tri;

    int j      = idx;
    while (j > 0 && swap_cond(queue[j], queue[j - 1])) {
        swap(Tri3f, queue[j], queue[j - 1]);
        j--;
    }
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

struct Circ {
    int x;
    int y;
    int r;
    float z;
    uint32 col;
};

void insert_circ(struct Circ queue[], int idx, struct Circ c)
{
    queue[idx] = c;

    int j      = idx;
    while (j > 0 && queue[j].z > queue[j - 1].z) {
        swap(struct Circ, queue[j], queue[j - 1]);
        j--;
    }
}

static float angle1 = 0;
void gfx_3d_test(GFX_Canvas canvas)
{
    angle1 += 0.02f;
    float s1        = fast_sin(angle1 * 0.7f);
    float c1        = fast_cos(angle1 * 0.7f);
    float s2        = fast_sin(angle1);
    float c2        = fast_cos(angle1);
    float s3        = fast_sin(angle1 * 0.5f);
    float c3        = fast_cos(angle1 * 0.5f);

    int grid_count  = 5;
    float grid_pad  = 0.5f / grid_count;
    float grid_size = ((grid_count - 1) * grid_pad);

    struct Circ queue[grid_count * grid_count * grid_count];
    int idx = 0;

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

                uint8 r  = (uint8)(((float)cx / (grid_count - 1)) * 255);
                uint8 g  = (uint8)(((float)cy / (grid_count - 1)) * 255);
                uint8 b  = (uint8)(((float)cz / (grid_count - 1)) * 255);

                insert_circ(queue, idx,
                    (struct Circ) {
                        .x   = px * canvas.px_w,
                        .y   = py * canvas.px_h,
                        .r   = 8 / canvas.scale,
                        .col = COL(r, g, b, 255),
                        .z   = z });
                idx += 1;
            }
        }
    }
    for (int i = 0; i < idx; i++) {
        gfx_fill_circ(canvas, queue[i].x, queue[i].y, queue[i].r, queue[i].col);
    }
}

void gfx_3d_test2(GFX_Canvas canvas)
{
    static float angle2 = 0.05f;
    Point3f camera      = { .z = -15.0f };
    Mat4f mProj         = chain_mul(2,
                get_proj_matrix(0.1f, 1000.0f, -120.0f, canvas.px_w, canvas.px_h),
                get_viewport_matrix());
    Mat4f mTrans        = get_trans_matrix(-camera.x, -camera.y, -camera.z);
    Mat4f mRotZ         = get_rotZ_matrix(-angle2 * 0.0f);
    Mat4f mRotY         = get_rotY_matrix(-angle2 * 1.0f);

    Tri3f draw_queue[(int)(sizeof(mesh) / sizeof(Tri3f))];
    int idx = 0;

    for (int i = 0; i < (int)(sizeof(mesh) / sizeof(Tri3f)); i++) {
        Tri3f proj, trans;

        Mat4f mTri = chain_mul(3,
            tri_to_mat4(mesh[i]),
            mRotZ,
            mRotY);
        mesh[i]    = mat4_to_tri(mTri);
        trans      = mesh[i];

        Point3f normal, l1, l2, pCamRay;
        l1      = p3f_sub(trans.vertex[1], trans.vertex[0]);
        l2      = p3f_sub(trans.vertex[2], trans.vertex[0]);
        normal  = p3f_cross(l1, l2);

        normal  = p3f_normalize(normal);
        pCamRay = p3f_sub(trans.vertex[0], camera);

        if (p3f_dot(normal, pCamRay) < 0) {
            Point3f light  = { .z = -1 };
            float dp       = p3f_dot(normal, light);

            mTri           = chain_mul(3, mTri, mTrans, mProj);
            proj           = mat4_to_tri(mTri);
            proj.vertex[0] = p3f_div(proj.vertex[0], proj.vertex[0].w);
            proj.vertex[1] = p3f_div(proj.vertex[1], proj.vertex[1].w);
            proj.vertex[2] = p3f_div(proj.vertex[2], proj.vertex[2].w);

            uint8 shade    = (uint8)(dp * 255);
            proj.col       = COL(shade, shade, shade, 255);

            insert(draw_queue, idx, proj);
            idx += 1;
        }
    }

    for (int i = 0; i < idx; i++) {
        gfx_fill_triangle(canvas,
            draw_queue[i].vertex[0].x * canvas.px_w, draw_queue[i].vertex[0].y * canvas.px_h,
            draw_queue[i].vertex[1].x * canvas.px_w, draw_queue[i].vertex[1].y * canvas.px_h,
            draw_queue[i].vertex[2].x * canvas.px_w, draw_queue[i].vertex[2].y * canvas.px_h,
            draw_queue[i].col);
        gfx_draw_triangle(canvas,
            draw_queue[i].vertex[0].x * canvas.px_w, draw_queue[i].vertex[0].y * canvas.px_h,
            draw_queue[i].vertex[1].x * canvas.px_w, draw_queue[i].vertex[1].y * canvas.px_h,
            draw_queue[i].vertex[2].x * canvas.px_w, draw_queue[i].vertex[2].y * canvas.px_h,
            0x000000FF);
    }
}
