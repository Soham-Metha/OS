/*
 * hal_browser.h
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
