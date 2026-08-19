/*
 * interrupt.h
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

#ifndef INTERRUPT_1
#define INTERRUPT_1

#include <common/types.h>

typedef enum Interrupt {
    IRQ_TIMER    = 0,
    IRQ_KEYBOARD = 1,
    IRQ_MOUSE    = 12,
    IRQ_COUNT,
} Interrupt;

typedef uint8 Timer_Freq;
typedef uint8 Keycode;

typedef struct Mouse_Movement {
    int dx;
    int dy;
    bool left;
    bool right;
    bool middle;
} Mouse_Movement;

typedef union IRQ_Data {
    Timer_Freq timer_freq;
    Keycode keycode;
    Mouse_Movement mouse_movement;
} IRQ_Data;

void kernel_irq(Interrupt i, IRQ_Data data);

#endif
