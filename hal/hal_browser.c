#include "hal.h"
#include "interrupt.h"

/*
 * This function is implemented in JavaScript
 * and imported into the WASM module.
 */

extern void __hal_write_char(int c);

void hal_write_char(char c)
{
    __hal_write_char((int)c);
}
