#define IMPL_TTY_1
#include "shell.h"
#include <userspace/services/tty.h>
#include <hal/hal.h>
#include <kernel/kernel.h>

#define COL(r, g, b, a) (r << 24 | g << 16 | b << 8 | a)

void kernel_tick(void)
{
    wm_render(&k.wm);
}

Compositor compositor = { 0 };
tty input_buffer      = { 0 };

int main(void)
{
    int screen_w = hal_get_width();
    int screen_h = hal_get_height();

    compositor_init(&compositor, screen_w, screen_h);
    wm_init(&k.wm, &compositor);

    Window* bg_win = wm_create_window(
        &k.wm, 0, 0,
        screen_w, screen_h,
        COL(0xFF, 0xFF, 0xFF, 0xFF), COL(0, 0, 0xFF, 0xFF));

    Window* win_2 = wm_create_window(
        &k.wm, 10, 10,
        200, 200,
        COL(0xFF, 0xFF, 0xFF, 0xFF), COL(0, 0xFF, 0, 0xFF));

    Window* win_3 = wm_create_window(
        &k.wm, 410, 10,
        380, 380,
        COL(0xFF, 0xFF, 0xFF, 0xFF), COL(0xFF, 0, 0, 0xFF));

    (void)bg_win;
    (void)win_2;
    tty_init(&input_buffer, &win_3->term);

    k.wm.fallback_tty = &input_buffer;

    tty_write_char(&input_buffer, 'H');
    tty_write_char(&input_buffer, 'i');
    tty_write_char(&input_buffer, '!');
    tty_write_char(&input_buffer, '\n');

    for (uint8 i = 0; i < win_3->term.cols; i++)
        tty_write_char(&input_buffer, '=');

    tty_flush(&input_buffer);

    return 0;
}
