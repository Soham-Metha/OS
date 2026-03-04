#define IMPL_FS_1
#define IMPL_GRAPHICS_1
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
#include <modules/graphics.h>
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
    GFX_Canvas canvas = GFX_CANVAS(s->pixels, s->width, s->height);
    GFX_Canvas sub_c1 = gfx_init_subcanvas(canvas, canvas.px_w / 2, 0, canvas.px_w / 2, canvas.px_h / 2);
    GFX_Canvas sub_c2 = gfx_init_subcanvas(canvas, 0, canvas.px_h / 2, canvas.px_w, canvas.px_h / 2);
    GFX_Canvas sub_c3 = gfx_init_subcanvas(canvas, 0, 0, canvas.px_w / 2, canvas.px_h / 2);
    gfx_fill(canvas, COL(0x11, 0x11, 0x11, 0xFF));
    gfx_fill(sub_c1, COL(0xFF, 0, 0, 0x88));
    gfx_fill(sub_c2, COL(0, 0xFF, 0, 0x88));
    gfx_fill(sub_c3, COL(0, 0, 0xFF, 0x88));

    gfx_pattern_checker(sub_c1,
        47, COL(0x22, 0x22, 0xFF, 0xFF), COL(0x11, 0x11, 0x11, 0xFF));
    gfx_pattern_circles(sub_c2,
        6, 11, COL(0xFF, 0x22, 0x22, 0xFF));
    gfx_pattern_shapes(sub_c3,
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
        COL(0xFF, 0xFF, 0xFF, 0xFF), COL(0, 0xFF, 0, 0xFF));
    shell_win    = wm_create_window(&wm, 0, 0, screen_w / 2, screen_h,
           COL(0xFF, 0xFF, 0xFF, 0xFF), COL(0, 0, 0, 0xFF));

    graphics_test(&graphics_win->surface);
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
