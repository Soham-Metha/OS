#include "shell.h"
#include <hal/hal.h>
#include <hal/interrupt.h>

#define COL(r, g, b, a) (r << 24 | g << 16 | b << 8 | a)

int main(void)
{
    int screen_width  = hal_get_width();
    int screen_height = hal_get_height();

    tty t             = { 0 };
    tty_init(&t,
        (screen_width / GLYPH_W), (screen_height / GLYPH_H),
        COL(0xFF, 0xFF, 0xFF, 0xFF), COL(0, 0, 0xFF, 0xFF));
    tty_write_char(&t, 'H');
    tty_write_char(&t, 'i');
    tty_write_char(&t, '!');
    tty_write_char(&t, '\n');

    for (uint8 i = 0; i < 225; i++)
        tty_write_char(&t, '=');

    tty_write_char(&t, '\n');

    return 0;
}
