#include "shell.h"
#include <hal/hal.h>
#include <hal/interrupt.h>

#define COL(r, g, b, a) (r << 24 | g << 16 | b << 8 | a)

int main(void)
{
    tty t = { 0 };
    tty_init(&t, 225, 55, COL(0xFF, 0xFF, 0xFF, 0xFF), COL(0, 0, 0xFF, 0xFF));
    tty_write_char(&t, 'H');
    tty_write_char(&t, 'i');
    tty_write_char(&t, '!');
    tty_write_char(&t, '\n');

    for (uint8 i = 0; i < 225; i++)
        tty_write_char(&t, '=');

    tty_write_char(&t, '\n');

    return 0;
}
