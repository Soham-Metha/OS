#include "interrupt.h"
#include "event.h"
// TODO: fix boundary violation
#include <userspace/shell.h>

void kernel_irq(Interrupt i, uint64 data)
{
    switch (i) {
    case IRQ_TIMER:
        kernel_tick();     // TODO: should be a kernel function not a userspace one
        break;
    case IRQ_KEYBOARD:
        kernel_event_enque((Event) { .type = EVENT_KEYBOARD, .as.key_event.keycode = data });
        break;
    case IRQ_COUNT:
    default:
        break;
    }
}
