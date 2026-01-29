#include "../hal.h"
#include <kernel/interrupt.h>
// #include <emscripten/wasm_worker.h>

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

void switch_to(void (*func)(void))
{
    // emscripten_wasm_worker_t worker = emscripten_malloc_wasm_worker(512);
    // emscripten_wasm_worker_post_function_v(worker, func);
    func();
}

void kernel_irq_wrapper(Interrupt i, int a, int b, int c)
{
    if (i == IRQ_TIMER)
        kernel_irq(i, (IRQ_Data) { .timer_freq = a });
    else if (i == IRQ_KEYBOARD)
        kernel_irq(i, (IRQ_Data) { .keycode = a });
    else if (i == IRQ_MOUSE)
        kernel_irq(i, (IRQ_Data) {
                          .mouse_movement = { .dx = a, .dy = b, .left = c & 1, .right = (c >> 1) & 1, .middle = (c >> 2) & 1 }
        });
}
