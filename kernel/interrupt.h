/* interrupt.h */

#ifndef INTERRUPT_1
#define INTERRUPT_1

#include <common/types.h>

typedef enum Interrupt {
    IRQ_KEYBOARD = 1,
    IRQ_COUNT,
} Interrupt;

void kernel_irq(Interrupt i, uint64 data);

#endif
