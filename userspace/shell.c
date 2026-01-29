#define IMPL_SCHEDULER_1
#define IMPL_TERMINAL_1
#define IMPL_TTY_1
#define IMPL_USPACE_IO_1
#define IMPL_WM_1
#include "shell.h"
#include "libs/io.h"
#include "services/wm.h"
#include <common/event.h>
// TODO: fix boundary violation
#include <drivers/tty.h>
#include <kernel/scheduler.h>

#define COL(r, g, b, a) (r << 24 | g << 16 | b << 8 | a)

WindowManager wm = { 0 };
Compositor comp  = { 0 };
tty io_buffer    = { 0 };
int screen_w     = { 0 };
int screen_h     = { 0 };

void event_handler(void)
{
    if (kernel_event_occurred()) {     // TODO: shouldn't directly access the kernel space, supposed to be a "userspace tick"
        Event e = kernel_event_deque();
        kernel_event_handler(e);
    }
    p_yield();
}

void renderer(void)
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
    (void)wm_create_window(&wm, 0, 0, screen_w, screen_h,
        COL(0xFF, 0xFF, 0xFF, 0xFF), COL(0, 0, 0xFF, 0xFF));

    (void)wm_create_window(&wm, 10, 10, 200, 200,
        COL(0xFF, 0xFF, 0xFF, 0xFF), COL(0, 0xFF, 0, 0xFF));

    Window* win_3 = wm_create_window(&wm, 410, 10, 380, 380,
        COL(0xFF, 0xFF, 0xFF, 0xFF), COL(0xFF, 0, 0, 0xFF));

    print_str("Shell v0.1\n");
    for (uint8 i = 0; i < win_3->term.cols; i++)
        putch('-');
    print_str("> ");
}

void shell(void)
{
    const char* user_str = getline();
    if (user_str[0] != '\0') {
        print_str(user_str);
        print_str("> ");
    }
    p_yield();     // TODO: improve context switching logic to allow pre-emption
}

extern int main(void)
{
    scheduler_init(kernel_init);
    create_task(event_handler);
    create_task(renderer);
    create_task(shell);
    return 0;
}
