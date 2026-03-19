#define IMPL_FS_1
#define IMPL_GRAPHICS_1
#define IMPL_GRAPHICS3D_1
#define IMPL_KERN_VIREX_1
#define IMPL_SCHEDULER_1
#define IMPL_TERMINAL_1
#define IMPL_USPACE_IO_1
#define IMPL_WM_1
#define MEM_MANAGER_IMPL
#define STRING_VIEW_IMPL
#include "shell.h"
#include "libs/io.h"
#include "services/wm.h"
#include <common/event.h>
#include <common/memmanager.h>
#include <common/strings.h>
#include <modules/graphics3d.h>
#include <modules/virex.h>
// TODO: fix boundary violation
#include <kernel/fs.h>
#include <kernel/scheduler.h>

WindowManager wm     = { 0 };
Compositor comp      = { 0 };
int screen_w         = { 0 };
int screen_h         = { 0 };
Window* graphics_win = { 0 };
Window* shell_win    = { 0 };

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

void event_handle_loop(void)
{
    if (event_occurred()) {
        Event e = event_deque();
        event_handler(e);
    }
    p_yield();
}

void render_loop(void)
{
    Result8 c = load(stdout);
    while (RESULT_OK(c)) {
        wm_handle_key(&wm, RESULT_VAL(c));
        c = load(stdout);
    }

    wm_render(&wm);
    p_yield();
}

void graphics_test(Surface* s)
{
    GFX_Canvas canvas  = GFX_CANVAS(s->pixels, s->width, s->height, 4);

    int xmax           = canvas.px_w;
    int ymax           = canvas.px_h;
    int xmid           = xmax / 2;
    int ymid           = ymax / 2;

    GFX_Canvas sub_c11 = gfx_init_subcanvas(canvas, 0, 0, xmid, ymid);
    GFX_Canvas sub_c12 = gfx_init_subcanvas(canvas, xmid, 0, xmid, ymid);
    GFX_Canvas sub_c2  = gfx_init_subcanvas(canvas, 0, ymid, xmax, ymid);

    gfx_fill(canvas, COL(0x11, 0x11, 0x11, 0xFF));

    gfx_3d_test(sub_c11);
    gfx_pattern_checker(sub_c12,
        47, COL(0x22, 0x22, 0xFF, 0xFF), COL(0x11, 0x11, 0x11, 0xFF));
    // gfx_pattern_circles(sub_c12,
    //     6, 11, COL(0xFF, 0x22, 0x22, 0xFF));
    gfx_pattern_shapes(sub_c2,
        COL(0x22, 0xFF, 0x22, 0xFF));
    s->dirty = true;
}

void kernel_init(void)
{
    screen_w = hal_get_width();
    screen_h = hal_get_height();

    compositor_init(&comp, screen_w, screen_h);
    wm_init(&wm, &comp);

    graphics_win = wm_create_window(&wm, screen_w / 2, 0, screen_w / 2, screen_h,
        COL(0xFF, 0xFF, 0xFF, 0xFF), COL(0, 0xFF, 0xFF, 0xFF));
    shell_win    = wm_create_window(&wm, 0, 0, screen_w / 2, screen_h,
           COL(0xFF, 0xFF, 0xFF, 0xFF), COL(0, 0, 0, 0xFF));
}

void fs_init(void)
{
    for (uint16 i = 0; i < (screen_w / 2) / GLYPH_W; i++)
        putch('-');
    print_str("File System v0.1\n");
    for (uint16 i = 0; i < (screen_w / 2) / GLYPH_W; i++)
        putch('-');

    filename dir_nm = (filename) { .name = "test" };
    filename fl_nm  = (filename) { .name = "myfile", .ext = "txt" };

    BlockDevice rd  = disk_init(DISK_RAMDISK, 64);

    ResultPtr r     = fs_format(&rd, 0);
    filesystem* fs  = (filesystem*)RESULT_VAL(r);

    ResultPtr d     = inode_create(fs, &dir_nm, DIR);
    ResultPtr f     = inode_create(fs, &fl_nm, FILE);

    if RESULT_ERR (r)
        printf("\nError when formatting disk : %d", r.error);
    if RESULT_ERR (d)
        printf("\nError when creating dir    : %d", d.error);
    if RESULT_ERR (f)
        printf("\nError when creating file   : %d", f.error);

    fs_show(fs, true);
}

void shell_win_init(void)
{
    for (uint16 i = 0; i < (screen_w / 2) / GLYPH_W; i++)
        putch('-');
    print_str("Shell v0.1\n");
    for (uint16 i = 0; i < (screen_w / 2) / GLYPH_W; i++)
        putch('-');

    print_str("\n> ");
}

void shell_handler(String_View inp)
{
    if (sv_compare(inp, STR("test"))) {
        virex_test();     // TODO: doesn't work in 'native' mode
    } else if (sv_compare(inp, STR("clear"))) {
        terminal_clear(&shell_win->term);
        shell_win_init();
        return;
    } else if (sv_compare(inp, STR("help"))) {
        printf("\nAvailable commands: ");
        printf("\n    help : display this help dialog");
        printf("\n    test : run the SASM test program");
        printf("\n    clear: clear shell terminal");
    } else {
        printf("\nInvalid command entered, use 'help' for a list of commands!");
        printf("\n    You entered: %s", inp.data);
    }
    printf("\n> ");
}

void shell_loop(void)
{
    ResultPtr r = getline();
    if RESULT_OK (r) {
        const char* str = (const char*)RESULT_VAL(r);
        shell_handler(STR(str));
    }
    graphics_test(&graphics_win->surface);
    p_yield();     // TODO: improve context switching logic to allow pre-emption
}

extern int main(void)
{
    scheduler_init(kernel_init);
    create_task(fs_init);
    create_task(shell_win_init);
    create_task(event_handle_loop);
    create_task(render_loop);
    create_task(shell_loop);
    return 0;
}
