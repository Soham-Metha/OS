/*
 * event.h
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
#include "types.h"

#ifndef EVENT_1
#define EVENT_1
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

#endif
