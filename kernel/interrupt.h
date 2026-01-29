/* interrupt.h */

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
