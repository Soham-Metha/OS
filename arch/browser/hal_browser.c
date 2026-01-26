#include "../hal.h"

/*
 * This function is implemented in JavaScript
 * and imported into the WASM module.
 */

extern void __hal_put_pixel(int x, int y, uint32 rgba);
extern void __hal_clear(uint32 rgba);
extern void __hal_present(void);
extern uint32 __hal_get_width(void);
extern uint32 __hal_get_height(void);

void hal_put_pixel(int x, int y, uint32 rgba)
{
    __hal_put_pixel(x, y, rgba);
}

void hal_clear(uint32 rgba)
{
    __hal_clear(rgba);
}

void hal_present(void)
{
    __hal_present();
}

uint32 hal_get_width()
{
    return __hal_get_width();
}

uint32 hal_get_height()
{
    return __hal_get_height();
}
