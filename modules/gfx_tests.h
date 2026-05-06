#include "graphics3d.h"
#include <examples/img_ex.c>
#include <examples/space.c>
#include <examples/space_tex.c>
#include <examples/fish.c>

// Examples available:
// ---- | ------------ | PX_PIKA   | CANVAS_PIKA   | 256x256
// ---- | ------------ | PX_BOY    | CANVAS_BOY    | 512x512
// ---- | ------------ | PX_CASTLE | CANVAS_CASTLE | 512x512
// ---- | ------------ | PX_MARIO  | CANVAS_MARIO  | 512x512
// ---- | ------------ | PX_SKY    | CANVAS_SKY    | 512x512
// ---- | ------------ | PX_SKY2   | CANVAS_SKY2   | 512x512
// ---- | ------------ | PX_SKY3   | CANVAS_SKY3   | 512x512
// ---- | ------------ | PX_METR   | CANVAS_METR   | 512x512
// ---- | ------------ | PX_CYBER  | CANVAS_CYBER  | 512x512
//  286 | MESH_GUN     | --------- | ------------- | -------
//  387 | MESH_FISH    | PX_FISH   | CANVAS_FISH   | 256x256
//  391 | MESH_CLOCK   | PX_CLOCK  | CANVAS_CLOCK  | 256x256
// 1613 | MESH_SHIP2   | PX_SPACE  | CANVAS_SPACE  | 512x512
// 2009 | MESH_PLANET1 | PX_SPACE  | CANVAS_SPACE  | 512x512
// 3375 | MESH_SHIP1   | PX_SPACE  | CANVAS_SPACE  | 512x512
// 5589 | MESH_ASTRO1  | PX_SPACE  | CANVAS_SPACE  | 512x512
// 5845 | MESH_MECH2   | PX_SPACE  | CANVAS_SPACE  | 512x512
// 6257 | MESH_ASTRO2  | PX_SPACE  | CANVAS_SPACE  | 512x512
// 6319 | MESH_TEA     | --------- | ------------- | -------

PX_SKY2;
static GFX_Canvas text = CANVAS_SKY2;

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

static Point3f camera   = { .z = -15.0f };
static float angle2     = 0.05f;
static Point3f vLookDir = { 0 };
static float angleY     = 0.00f;

void gfx_3d_Cam_Move(float tx, float ty, float tz, float ry)
{
    camera.x += tx;
    camera.y += ty;
    angleY += ry;
    Point3f fwd = p3f_mul(vLookDir, tz);
    camera      = p3f_add(camera, fwd);
}

void gfx_3d_test2(GFX_Canvas canvas)
{
    angle2 += 0.05f;
    PX_SPACE;
    static GFX_Canvas tex   = CANVAS_SPACE;
    static Tri3f mesh[]     = MESH_SHIP2;
    static Point3f light    = { .z = -1 };

    static bool initialized = false;
    static Mat4f mTransProjView;
    if (!initialized) {
        // Fixed operations
        // move object away from camera before render
        // project 3d co ords into 2d co ords
        // view port transforms to get it into 0-1 range
        mTransProjView = matrix_chain(3,
            matrix_trans(0, 0, 1.0f),
            matrix_project(0.01f, 250.0f, 90.0f, canvas.px_w, canvas.px_h),
            matrix_viewport());
        initialized    = true;
    }

    static Tri3f draw_queue[(int)(sizeof(mesh) / sizeof(Tri3f)) * 2];
    int idx      = 0;

    vLookDir     = p3f_mul_mat((Point3f) { .z = 1.0f }, matrix_rotY(angleY));

    // Dynamic operations
    // the rotation angle changes each frame
    // camera position may change based on user input
    Mat4f mWorld = matrix_chain(3,
        matrix_rotZ(-angle2 * 0.5f),
        matrix_rotY(-angle2 * 1.0f),
        matrix_inv(matrix_pointed(
            camera,
            p3f_add(camera, vLookDir),
            (Point3f) { .y = 1 })));

    // TODO: switch from a per-face loop to a per-vertex loop?
    // TODO: since vertices may be shared, would improve performance?
    for (int i = 0; i < (int)(sizeof(mesh) / sizeof(Tri3f)); i++) {
        Tri3f trans = {
            .vertex[0]  = p3f_mul_mat(mesh[i].vertex[0], mWorld),
            .vertex[1]  = p3f_mul_mat(mesh[i].vertex[1], mWorld),
            .vertex[2]  = p3f_mul_mat(mesh[i].vertex[2], mWorld),
            .texture[0] = mesh[i].texture[0],
            .texture[1] = mesh[i].texture[1],
            .texture[2] = mesh[i].texture[2],
        };

        Point3f pCamRay = p3f_sub(trans.vertex[0], camera);
        Point3f normal  = p3f_normalize(
             p3f_cross(
                 p3f_sub(trans.vertex[1], trans.vertex[0]),
                 p3f_sub(trans.vertex[2], trans.vertex[0])));

        if (p3f_dot(normal, pCamRay) >= 0)
            continue;

        float dp = p3f_dot(normal, light);
        if (dp > 1.0f) dp = 1.0f;
        if (dp < 0.1f) dp = 0.1f;
        trans.shade = dp;

        Tri3f clipped[2];
        uint8 clip_cnt = tri_clip(
            (Point3f) { .z = 0.01f },
            (Point3f) { .z = 1.0f },
            trans,
            &clipped[0],
            &clipped[1]);

        for (uint8 j = 0; j < clip_cnt; j++) {

            // Translate, Project and Viewport
            Tri3f proj = {
                .shade      = clipped[j].shade,
                .vertex[0]  = p3f_mul_mat(clipped[j].vertex[0], mTransProjView),
                .vertex[1]  = p3f_mul_mat(clipped[j].vertex[1], mTransProjView),
                .vertex[2]  = p3f_mul_mat(clipped[j].vertex[2], mTransProjView),
            };

            proj.texture[0] = p2f_div(clipped[j].texture[0], proj.vertex[0].w);
            proj.texture[1] = p2f_div(clipped[j].texture[1], proj.vertex[1].w);
            proj.texture[2] = p2f_div(clipped[j].texture[2], proj.vertex[2].w);

            proj.texture[0].w = 1.0f / proj.vertex[0].w;
            proj.texture[1].w = 1.0f / proj.vertex[1].w;
            proj.texture[2].w = 1.0f / proj.vertex[2].w;

            // TODO: should p3f/p2f propogate w? currently they dont
            proj.vertex[0] = p3f_div(proj.vertex[0], proj.vertex[0].w);
            proj.vertex[1] = p3f_div(proj.vertex[1], proj.vertex[1].w);
            proj.vertex[2] = p3f_div(proj.vertex[2], proj.vertex[2].w);

            Tri3f tri_q[8];
            int q_count      = 0;
            tri_q[q_count++] = proj;

            for (int p = 0; p < 4; p++) {
                Tri3f new_q[8];
                int new_q_cnt = 0;

                for (int ii = 0; ii < q_count; ii++) {
                    Tri3f clip[2];
                    int n = 0;

                    switch (p) {
                    case 0: n = tri_clip((Point3f) { .y = 0 }, (Point3f) { .y = +1 }, tri_q[ii], &clip[0], &clip[1]); break;
                    case 1: n = tri_clip((Point3f) { .y = 1 }, (Point3f) { .y = -1 }, tri_q[ii], &clip[0], &clip[1]); break;
                    case 2: n = tri_clip((Point3f) { .x = 0 }, (Point3f) { .x = +1 }, tri_q[ii], &clip[0], &clip[1]); break;
                    case 3: n = tri_clip((Point3f) { .x = 1 }, (Point3f) { .x = -1 }, tri_q[ii], &clip[0], &clip[1]); break;
                    }

                    for (int k = 0; k < n; k++)
                        new_q[new_q_cnt++] = clip[k];
                }

                memcpy(tri_q, new_q, sizeof(Tri3f) * new_q_cnt);
                q_count = new_q_cnt;
            }

            for (int i = 0; i < q_count; i++) {
                insert(draw_queue, idx, tri_q[i]);
                idx += 1;
            }
        }
    }

    for (int i = 0; i < idx; i++) {
        // uint8 shade255 = draw_queue[i].shade * 255;
        // uint32 col     = COL(shade255, shade255, shade255, 0xFF);
        // gfx_fill_triangle(canvas,
        //     draw_queue[i].vertex[0].x * canvas.px_w, draw_queue[i].vertex[0].y * canvas.px_h,
        //     draw_queue[i].vertex[1].x * canvas.px_w, draw_queue[i].vertex[1].y * canvas.px_h,
        //     draw_queue[i].vertex[2].x * canvas.px_w, draw_queue[i].vertex[2].y * canvas.px_h,
        //     col);
        gfx_fill_textured_triangle(canvas,
            tex, draw_queue[i].texture,
            draw_queue[i].vertex[0].x * canvas.px_w, draw_queue[i].vertex[0].y * canvas.px_h,
            draw_queue[i].vertex[1].x * canvas.px_w, draw_queue[i].vertex[1].y * canvas.px_h,
            draw_queue[i].vertex[2].x * canvas.px_w, draw_queue[i].vertex[2].y * canvas.px_h,
            draw_queue[i].shade);
        gfx_draw_triangle(canvas,
            draw_queue[i].vertex[0].x * canvas.px_w, draw_queue[i].vertex[0].y * canvas.px_h,
            draw_queue[i].vertex[1].x * canvas.px_w, draw_queue[i].vertex[1].y * canvas.px_h,
            draw_queue[i].vertex[2].x * canvas.px_w, draw_queue[i].vertex[2].y * canvas.px_h,
            0x000000FF);
    }
}

void gfx_testt(GFX_Canvas canvas)
{
    gfx_fill_textured(canvas, text);
}
