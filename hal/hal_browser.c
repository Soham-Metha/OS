#include "hal.h"
#include "interrupt.h"

/*
 * These functions are implemented in JavaScript
 * and imported into the WASM module.
 */

extern int __hal_read_char(void);
extern void __hal_write_char(int c);

char hal_read_char(void)
{
    int r = tty_read_char();
    return (char)r;
}

void hal_write_char(char c)
{
    __hal_write_char((int)c);
}
