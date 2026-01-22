#include "interrupt.h"

void kernel_irq(Interrupt i, uint64 data)
{
    switch (i) {
    case IRQ_KEYBOARD:
        tty_push_key(active_tty, data);
        break;
    case IRQ_COUNT:
    default:
        break;
    }
}
