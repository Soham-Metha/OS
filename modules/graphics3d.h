#ifndef GRAPHICS3D_1
#define GRAPHICS3D_1
#include "graphics.h"
#include "math.h"

typedef struct {
    float x;
    float y;
    float z;
    float w;
} Point3f;

typedef struct {
    Point3f vertex[3];
    Point3f normal[3];
    uint32 col;
} Tri3f;

typedef struct
{
    float m[4][4];
} Mat4f;

void gfx_3d_draw_point(GFX_Canvas canvas, int x, int y, int z, int r, uint32 col);
void gfx_3d_draw_line(GFX_Canvas canvas, int x1, int y1, int z1, int x2, int y2, int z2, uint32 col);
void gfx_3d_draw_cube(GFX_Canvas canvas, int x1, int y1, int z1, int x2, int y2, int z2, uint32 col);
Mat4f tri_to_mat4(Tri3f t);
Point3f p3f_sub(Point3f a, Point3f b);

Tri3f mat4_to_tri(Mat4f m);
Mat4f mat_mul(Mat4f A, Mat4f B);
Mat4f chain_mul(int count, ...);

Mat4f get_proj_matrix(float z_min, float z_max, float fov, int w, int h);
Mat4f get_viewport_matrix();

Mat4f get_trans_matrix(float x, float y, float z);
Mat4f get_scale_matrix(float x, float y, float z);
Mat4f get_rotX_matrix(float angle);
Mat4f get_rotY_matrix(float angle);
Mat4f get_rotZ_matrix(float angle);

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

float p3f_dot(Point3f p1, Point3f p2)
{
    return p1.x * p2.x + p1.y * p2.y + p1.z * p2.z;
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

float p3f_length(Point3f a)
{
    return fast_sqrt(p3f_dot(a, a));
}

Point3f p3f_normalize(Point3f a)
{
    float len = p3f_length(a);
    return (Point3f) {
        .x = a.x / len,
        .y = a.y / len,
        .z = a.z / len,
    };
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

Mat4f tri_to_mat4(Tri3f t)
{
    return (Mat4f) {
        // P1
        .m[0][0] = t.vertex[0].x,
        .m[0][1] = t.vertex[0].y,
        .m[0][2] = t.vertex[0].z,
        .m[0][3] = t.vertex[0].w,
        // P2
        .m[1][0] = t.vertex[1].x,
        .m[1][1] = t.vertex[1].y,
        .m[1][2] = t.vertex[1].z,
        .m[1][3] = t.vertex[1].w,
        // P3
        .m[2][0] = t.vertex[2].x,
        .m[2][1] = t.vertex[2].y,
        .m[2][2] = t.vertex[2].z,
        .m[2][3] = t.vertex[2].w,
        // Dummy
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

Mat4f mat_mul(Mat4f A, Mat4f B)
{
    (void)A;
    (void)B;
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

Mat4f chain_mul(int count, ...)
{
    va_list args;
    va_start(args, count);

    Mat4f result = va_arg(args, Mat4f);     // first matrix

    for (int i = 1; i < count; i++) {
        Mat4f m = va_arg(args, Mat4f);
        result  = mat_mul(result, m);
    }

    va_end(args);
    return result;
}

Mat4f get_proj_matrix(float z_min, float z_max, float fov, int w, int h)
{
    float aspect_ratio = (float)h / w;
    float fov_tan      = 1.0f / fast_tan(fov);
    return (Mat4f) {
        .m[0][0] = aspect_ratio * fov_tan,
        .m[1][1] = fov_tan,
        .m[2][2] = z_max / (z_max - z_min),
        .m[3][2] = (-z_max * z_min) / (z_max - z_min),
        .m[2][3] = 1.0f,
        .m[3][3] = 0.0f,
    };
}

Mat4f get_viewport_matrix()
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

Mat4f get_trans_matrix(float x, float y, float z)
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

Mat4f get_scale_matrix(float x, float y, float z)
{
    return (Mat4f) {
        .m[0][0] = x,
        .m[1][1] = y,
        .m[2][2] = z,
        .m[3][3] = 1,
    };
}

Mat4f get_rotX_matrix(float angle)
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

Mat4f get_rotY_matrix(float angle)
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

Mat4f get_rotZ_matrix(float angle)
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

#endif
