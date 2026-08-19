/*
 * interrupt.c
 *  Copyright (C) 2026 Soham Metha
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
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
