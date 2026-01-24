/* event.h */
#include <common/types.h>

typedef enum EventType {
    EVENT_KEYBOARD,
    EVENT_COUNT,
} EventType;

typedef struct KeyEvent {
    uint64 keycode;
} KeyEvent;

typedef union EventData {
    KeyEvent key_event;
} EventData;

typedef struct Event {
    EventType type;
    EventData as;
} Event;

void kernel_event_enque(Event e);
Event kernel_event_deque();
bool kernel_event_occurred();
void kernel_event_handler(Event e);
