#include "interrupt.h"
#include "hal.h"

static uint8 buf[TTY_BUF_SIZE];
static uint8 head = 0, tail = 0;


void tty_push_key(uint8 c)
{
    buf[head] = c;
    head      = (head + 1) % TTY_BUF_SIZE;
    hal_write_char(c);
}

uint8 tty_read_char(void)
{
    if (head == tail)
        return -1;
    char c = buf[tail];
    tail   = (tail + 1) % TTY_BUF_SIZE;
    return c;
}
