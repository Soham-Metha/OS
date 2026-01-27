#include "interrupt.h"
#include "event.h"
#include "scheduler.h"

void kernel_irq(Interrupt i, uint64 data)
{
    switch (i) {
    case IRQ_TIMER:
        resume(TASK_WAITING);
        schedule();
        break;
    case IRQ_KEYBOARD:
        kernel_event_enque((Event) { .type = EVENT_KEYBOARD, .as.key_event.keycode = data });
        resume(TASK_BLOCKED);
        break;
    case IRQ_COUNT:
    default:
        break;
    }
}
