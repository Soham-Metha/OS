#define IMPL_FS_1
#define IMPL_SCHEDULER_1
#define IMPL_TERMINAL_1
#define IMPL_USPACE_IO_1
#define IMPL_WM_1
#define MEM_MANAGER_IMPL
#include "shell.h"
#include "libs/io.h"
#include "services/wm.h"
#include <common/event.h>
#include <common/memmanager.h>
// TODO: fix boundary violation
#include <kernel/fs.h>
#include <kernel/scheduler.h>

#define COL(r, g, b, a) (r << 24 | g << 16 | b << 8 | a)

WindowManager wm = { 0 };
Compositor comp  = { 0 };
int screen_w     = { 0 };
int screen_h     = { 0 };

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
    while
        RESULT_OK(c)
        {
            wm_handle_key(&wm, RESULT_VAL(c));
            c = load(stdout);
        }

    wm_render(&wm);
    p_yield();
}

void kernel_init(void)
{
    screen_w = hal_get_width();
    screen_h = hal_get_height();

    compositor_init(&comp, screen_w, screen_h);
    wm_init(&wm, &comp);

    (void)wm_create_window(&wm, 0, 0, screen_w, screen_h,
        COL(0xFF, 0xFF, 0xFF, 0xFF), COL(0, 0, 0, 0xFF));
}

void fs_init(void)
{
    for (uint16 i = 0; i < screen_w / GLYPH_W; i++)
        putch('-');
    print_str("File System v0.1\n");
    for (uint16 i = 0; i < screen_w / GLYPH_W; i++)
        putch('-');

    filename dir_nm  = (filename) { .name = "test" };
    filename fl_nm   = (filename) { .name = "myfile", .ext = "txt" };

    BlockDevice rd   = disk_init(DISK_RAMDISK, 64);

    ResultPtr r      = fs_format(&rd, 0);
    filesystem* fs   = (filesystem*)RESULT_VAL(r);

    ResultPtr d      = inode_create(fs, &dir_nm, DIR);
    ResultPtr f      = inode_create(fs, &fl_nm, FILE);

    if RESULT_ERR (r) printf("\nError when formatting disk : %d", r.error);
    if RESULT_ERR (d) printf("\nError when creating dir    : %d", d.error);
    if RESULT_ERR (f) printf("\nError when creating file   : %d", f.error);

    fs_show(fs, true);
}

void shell_win_init(void)
{
    for (uint16 i = 0; i < screen_w / GLYPH_W; i++)
        putch('-');
    print_str("Shell v0.1\n");
    for (uint16 i = 0; i < screen_w / GLYPH_W; i++)
        putch('-');

    print_str("\n> ");
}

void shell_loop(void)
{
    ResultPtr r = getline();
    if RESULT_OK (r) {
        printf("%s\n> ", (const char*)RESULT_VAL(r));
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
