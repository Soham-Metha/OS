#include <common/event.h>
#include <userspace/services/wm.h>

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
    case EVENT_COUNT:
    default:
        break;
    }
}
