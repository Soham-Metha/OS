#define IMPL_SCHEDULER_1
#define IMPL_TERMINAL_1
#define IMPL_TTY_1
#define IMPL_USPACE_IO_1
#define IMPL_WM_1
#define MEM_MANAGER_IMPL
#include "shell.h"
#include "libs/io.h"
#include "services/wm.h"
#include <common/event.h>
#include <common/memmanager.h>
// TODO: fix boundary violation
#include <drivers/tty.h>
#include <kernel/scheduler.h>

#define COL(r, g, b, a) (r << 24 | g << 16 | b << 8 | a)

WindowManager wm = { 0 };
Compositor comp  = { 0 };
tty io_buffer    = { 0 };
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
    uint8 buf[256];
    int char_read = tty_read_out(&io_buffer, buf, 256);     // TODO: replace with read syscall
    for (int i = 0; i < char_read; i++) {
        wm_handle_key(&wm, buf[i]);
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
    tty_init(&io_buffer);
    wm.fallback_tty = &io_buffer;

    (void)wm_create_window(&wm, 0, 0, screen_w / 2, screen_h / 2,
        COL(0xFF, 0xFF, 0xFF, 0xFF), COL(0, 0, 0xFF, 0xFF));

    (void)wm_create_window(&wm, screen_w / 2, 0, screen_w / 2, screen_h / 2,
        COL(0xFF, 0xFF, 0xFF, 0xFF), COL(0, 0xFF, 0, 0xFF));

    (void)wm_create_window(&wm, 0, screen_h / 2, screen_w / 2, screen_h / 2,
        COL(0xFF, 0xFF, 0xFF, 0xFF), COL(0xFF, 0, 0, 0xFF));

    Window* win_4 = wm_create_window(&wm, screen_w / 2, screen_h / 2, screen_w / 2, screen_h / 2,
        COL(0xFF, 0xFF, 0xFF, 0xFF), COL(0, 0, 0, 0xFF));

    print_str("Shell v0.1\n");
    for (uint8 i = 0; i < win_4->term.cols; i++)
        putch('-');
    print_str("> ");
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
    create_task(event_handle_loop);
    create_task(render_loop);
    create_task(shell_loop);
    return 0;
}
