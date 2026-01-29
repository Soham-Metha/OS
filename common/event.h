/* event.h */
#include <common/types.h>

typedef enum EventType {
    EVENT_MOUSE,
    EVENT_KEYBOARD,
    EVENT_COUNT,
} EventType;

typedef struct KeyEvent {
    uint64 keycode;
} KeyEvent;

typedef struct MouseEvent {
    int dx;
    int dy;
    bool left;
    bool right;
    bool middle;
    bool dirty;
} MouseEvent;

typedef union EventData {
    KeyEvent key_event;
    MouseEvent mouse_event;
} EventData;

typedef struct Event {
    EventType type;
    EventData as;
} Event;

/* MUST BE DEFINED IN USERSPACE */
void event_enque(Event e);
Event event_deque();
bool event_occurred();
void event_handler(Event e);
