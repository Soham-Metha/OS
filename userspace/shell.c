#define IMPL_TERMINAL_1
#define IMPL_TTY_1
#define IMPL_WM_1
#include "shell.h"
#include <userspace/services/wm.h>
// TODO: fix boundary violation
#include <drivers/tty.h>
#include <kernel/event.h>
#include <kernel/kernel.h>

#define COL(r, g, b, a) (r << 24 | g << 16 | b << 8 | a)

WindowManager wm = { 0 };
Compositor comp  = { 0 };
tty io_buffer    = { 0 };
int screen_w     = { 0 };
int screen_h     = { 0 };

void kernel_tick(void)
{
    if (kernel_event_occurred()) { // TODO: shouldn't directly access the kernel space, supposed to be a "userspace tick"
        Event e = kernel_event_deque();
        kernel_event_handler(e);
    }

    uint8 buf[256];
    int char_read = tty_read_out(&io_buffer, buf, 256);     // TODO: replace with read syscall
    for (int i = 0; i < char_read; i++) {
        wm_handle_key(&wm, buf[i]);
    }

    wm_render(&wm);
}

void kernel_init(void)
{
    screen_w = hal_get_width();
    screen_h = hal_get_height();

    compositor_init(&comp, screen_w, screen_h);
    wm_init(&wm, &comp);
    tty_init(&io_buffer);
    wm.fallback_tty = &io_buffer;
}

extern int main(void)
{
    kernel_init();
    (void)wm_create_window(&wm, 0, 0, screen_w, screen_h,
        COL(0xFF, 0xFF, 0xFF, 0xFF), COL(0, 0, 0xFF, 0xFF));

    (void)wm_create_window(&wm, 10, 10, 200, 200,
        COL(0xFF, 0xFF, 0xFF, 0xFF), COL(0, 0xFF, 0, 0xFF));

    Window* win_3 = wm_create_window(&wm, 410, 10, 380, 380,
        COL(0xFF, 0xFF, 0xFF, 0xFF), COL(0xFF, 0, 0, 0xFF));

    tty_write_char(&io_buffer, 'H');
    tty_write_char(&io_buffer, 'i');
    tty_write_char(&io_buffer, '!');
    tty_write_char(&io_buffer, '\n');

    for (uint8 i = 0; i < win_3->term.cols; i++)
        tty_write_char(&io_buffer, '=');

    return 0;
}
