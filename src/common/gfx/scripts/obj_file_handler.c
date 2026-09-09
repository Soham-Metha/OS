#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VERTS 100000
#define MAX_LINE 256

typedef struct {
    float u, v;
} Point2f;

typedef struct {
    float x, y, z, w;
} Point3f;

typedef struct {
    Point3f vertex[3];
    Point3f normal[3];
    Point2f texture[3];
} Tri3f;

Point3f verts[MAX_VERTS];
Point3f norms[MAX_VERTS];
Point2f uvs[MAX_VERTS];

int vert_count = 0;
int norm_count = 0;
int trin_count = 0;
int uv_count   = 0;

/**
 * Simple script to convert Obj files into static tables for 3d graphics!
 * NOTE: Obj file must be triangulated
 */

int main()
{
    FILE* f = fopen("fish.obj", "r");
    if (!f) {
        perror("open failed");
        return 1;
    }

    char line[MAX_LINE];

    printf("#define FISH_MESH {   \\\n");

    while (fgets(line, sizeof(line), f)) {

        // ---- vertex ----
        if (strncmp(line, "v ", 2) == 0) {
            Point3f* v = &verts[vert_count++];
            sscanf(line, "v %f %f %f", &v->x, &v->y, &v->z);
        }

        // ---- normal ----
        else if (strncmp(line, "vn ", 3) == 0) {
            Point3f* n = &norms[norm_count++];
            sscanf(line, "vn %f %f %f", &n->x, &n->y, &n->z);
        }

        // ---- tex ----
        else if (strncmp(line, "vt ", 3) == 0) {
            Point2f* t = &uvs[uv_count++];
            sscanf(line, "vt %f %f", &t->u, &t->v);
        }

        // ---- face ----
        else if (strncmp(line, "f ", 2) == 0) {

            int v[3]    = { 0 };
            int vt[3]   = { -1, -1, -1 };
            int vn[3]   = { -1, -1, -1 };

            // Try: f v/vt/vn
            int matches = sscanf(line,
                "f %d/%d/%d %d/%d/%d %d/%d/%d",
                &v[0], &vt[0], &vn[0],
                &v[1], &vt[1], &vn[1],
                &v[2], &vt[2], &vn[2]);

            if (matches != 9) {
                // Try: f v//vn
                matches = sscanf(line,
                    "f %d//%d %d//%d %d//%d",
                    &v[0], &vn[0],
                    &v[1], &vn[1],
                    &v[2], &vn[2]);
            }

            if (matches < 6) {
                matches = sscanf(line,
                    "f %d/%d %d/%d %d/%d",
                    &v[0], &vt[0],
                    &v[1], &vt[1],
                    &v[2], &vt[2]);
            }

            // Fetch vertices
            Point3f p[3];
            for (int i = 0; i < 3; i++) {
                p[i] = verts[v[i] - 1];
            }

            // Fetch uv (or default 0)
            Point2f t[3] = { 0 };
            for (int i = 0; i < 3; i++) {
                if (vt[i] != -1) {
                    t[i]   = uvs[vt[i] - 1];
                    t[i].v = 1.0f - t[i].v;     // Blender user bottom-up texturig
                }
            }

            // Fetch normals (or default 0)
            Point3f n[3] = { { 0 } };
            for (int i = 0; i < 3; i++) {
                if (vn[i] != -1) {
                    n[i] = norms[vn[i] - 1];
                }
            }

            // Print triangle
            printf(
                "    [%d] = { .vertex = {"
                "{ .x=%f, .y=%f, .z=%f, .w=1.0f },"
                "{ .x=%f, .y=%f, .z=%f, .w=1.0f },"
                "{ .x=%f, .y=%f, .z=%f, .w=1.0f }"
                "},"
                "    .texture = {"
                "{ .u=%f, .v=%f },"
                "{ .u=%f, .v=%f },"
                "{ .u=%f, .v=%f }"
                "},"
                // "    .normal = {"
                // "{ .x=%f, .y=%f, .z=%f, .w=1.0f },"
                // "{ .x=%f, .y=%f, .z=%f, .w=1.0f },"
                // "{ .x=%f, .y=%f, .z=%f, .w=1.0f },"
                // "},"
                "},\\\n",
                trin_count,
                p[0].x, p[0].y, p[0].z,
                p[1].x, p[1].y, p[1].z,
                p[2].x, p[2].y, p[2].z,
                t[0].u, t[0].v,
                t[1].u, t[1].v,
                t[2].u, t[2].v
                // n[0].x, n[0].y, n[0].z,
                // n[1].x, n[1].y, n[1].z,
                // n[2].x, n[2].y, n[2].z
            );
            trin_count += 1;
        }
    }

    printf("}\n");

    fclose(f);
    return 0;
}