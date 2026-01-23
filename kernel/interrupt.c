#include "interrupt.h"
#include <drivers/wm.h>
#include <kernel/kernel.h>

void kernel_irq(Interrupt i, uint64 data)
{
    switch (i) {
    case IRQ_KEYBOARD:
        wm_handle_key(&k.wm, data);
        break;
    case IRQ_COUNT:
    default:
        break;
    }
}
