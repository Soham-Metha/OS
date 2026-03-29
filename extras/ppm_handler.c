#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int uint32;
#define COL(r, g, b, a) (r << 24 | g << 16 | b << 8 | a)

int main()
{
    FILE* f = fopen("fish_tex.ppm", "rb");     // IMPORTANT: binary mode
    if (!f) {
        perror("open failed");
        return 1;
    }
    char magic[3];
    fscanf(f, "%2s", magic);

    if (strcmp(magic, "P6") != 0) {
        fprintf(stderr, "Only P6 supported\n");
        return 1;
    }
    int w, h, maxval;
    fscanf(f, "%d %d", &w, &h);
    fscanf(f, "%d", &maxval);
    fgetc(f);

    printf("#define FISH_PX static uint32 img_px[%d] = { \\\n", w * h);

    for (int i = 0; i < w * h; i++) {
        unsigned char rgb[3];

        if (fread(rgb, 1, 3, f) != 3) {
            fprintf(stderr, "Unexpected EOF\n");
            return 1;
        }

        int r = rgb[0];
        int g = rgb[1];
        int b = rgb[2];

        if (maxval != 255) {
            r = (r * 255) / maxval;
            g = (g * 255) / maxval;
            b = (b * 255) / maxval;
        }

        uint32 px = COL(r, g, b, 0xFF);

        printf("0x%08X,", px);

        if ((i + 1) % w == 0)
            printf("\\\n");
        else
            printf(" ");
    }

    printf("}\n\n");

    printf("#define FISH_CANVAS { \\\n");
    printf("    .px = img_px,");
    printf("    .px_w = %d,", w);
    printf("    .px_h = %d,", h);
    printf("    .px_stride = %d,", w);
    printf("    .scale = 1.0f,");
    printf("}\n");

    fclose(f);
    return 0;
}