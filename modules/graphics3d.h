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

float p3f_len(Point3f a);
float p3f_dot(Point3f a, Point3f b);
Point3f p3f_cross(Point3f a, Point3f b);
Point3f p3f_normalize(Point3f a);
Point3f p3f_mul_mat(Point3f i, Mat4f m);

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

#endif

#ifdef IMPL_GRAPHICS3D_1
#undef IMPL_GRAPHICS3D_1
#define SCALE_CONST 1

float p3f_dot(Point3f a, Point3f b)
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

float p3f_len(Point3f a)
{
    return fast_sqrt(p3f_dot(a, a));
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

#endif
