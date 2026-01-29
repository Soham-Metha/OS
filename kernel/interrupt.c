#include "interrupt.h"
#include "event.h"
#include "kernel.h"
#include "scheduler.h"
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
            if (data.keycode < 128 && scan_code_ascii[data.keycode] >= COUNT)
                tty_push_key(k.active_tty, scan_code_ascii[data.keycode]);
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
            e.as.mouse_event.dirty  = (data.mouse_movement.dx != 0) || (data.mouse_movement.dy != 0);
            kernel_event_enque(e);
        }
    case IRQ_COUNT:
    default:
        break;
    }
}
