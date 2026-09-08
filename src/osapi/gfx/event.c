/*
 * event.c
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
#include "wm.h"
#include <common/event.h>
#include <osapi/osapi.h>

Event event_queue[256];
uint8 head = 0;
uint8 tail = 0;

void event_enque(Event e)
{
    event_queue[head] = e;
    head              = (head + 1) % 256;
}

Event event_deque()
{
    Event out = (Event) { .type = EVENT_COUNT };
    if (event_occurred()) {
        out  = event_queue[tail];
        tail = (tail + 1) % 256;
    }
    return out;
}

bool event_occurred()
{
    return head != tail;
}

void event_handler(Event e)
{
    switch (e.type) {
    case EVENT_MOUSE:
        wm_handle_mouse(&wm, e.as.mouse_event);
        return;
    case EVENT_KEYBOARD:
        store(stdin, e.as.key_event.keycode);
        return;
    case EVENT_COUNT:
    default:
        break;
    }
}
