#include "interrupt.h"
#include "kernel.h"
#include "scheduler.h"
#include <common/event.h>
#include <drivers/keyboard.h>

void kernel_irq(Interrupt i, IRQ_Data data)
{
    switch (i) {
    case IRQ_TIMER:
        resume(TASK_WAITING);
        schedule();
        break;
    case IRQ_KEYBOARD:
        {
            if (data.keycode >= 128 || scan_code_ascii[data.keycode] < 1)
                return;
            Event e;
            e.type                 = EVENT_KEYBOARD;
            e.as.key_event.keycode = scan_code_ascii[data.keycode];
            event_enque(e);
            resume(TASK_BLOCKED);
        }
        break;
    case IRQ_MOUSE:
        {
            Event e;
            e.type                  = EVENT_MOUSE;
            e.as.mouse_event.dx     = data.mouse_movement.dx;
            e.as.mouse_event.dy     = data.mouse_movement.dy;
            e.as.mouse_event.left   = data.mouse_movement.left;
            e.as.mouse_event.right  = data.mouse_movement.right;
            e.as.mouse_event.middle = data.mouse_movement.middle;
            event_enque(e);
        }
    case IRQ_COUNT:
    default:
        break;
    }
}
