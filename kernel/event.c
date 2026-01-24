#include "event.h"
#include "kernel.h"
// TODO: fix boundary violation
#include <userspace/services/wm.h>

Event event_queue[256];
uint8 head = 0;
uint8 tail = 0;

void kernel_event_enque(Event e)
{
    event_queue[head] = e;
    head              = (head + 1) % 256;
}

Event kernel_event_deque()
{
    Event out = (Event) { .type = EVENT_COUNT };
    if (kernel_event_occurred()) {
        out  = event_queue[tail];
        tail = (tail + 1) % 256;
    }
    return out;
}

bool kernel_event_occurred()
{
    return head != tail;
}

void kernel_event_handler(Event e)
{
    switch (e.type) {
    case EVENT_KEYBOARD:
        wm_handle_key(&k.wm, e.as.key_event.keycode);
        break;

    case EVENT_COUNT:
    default:
        break;
    }
}
