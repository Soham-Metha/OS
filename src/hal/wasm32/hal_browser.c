/*
 * hal_browser.c
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
#include <kernel/heap.h>
// #include <emscripten/wasm_worker.h>

static uint32* fb_px;
static GFX_Canvas frame_buffer;

/*
 * This function is implemented in JavaScript
 * and imported into the WASM module.
 */

extern void __hal_present(uint32* pixels, uint32 width, uint32 height, uint32 stride);

static inline uint32 rgba_to_abgr(uint32 rgba)
{
    uint32 r = (rgba >> 24) & 0xFF;
    uint32 g = (rgba >> 16) & 0xFF;
    uint32 b = (rgba >> 8)  & 0xFF;
    uint32 a = rgba & 0xFF;

    return (a << 24) | (b << 16) | (g << 8) | r;
}

void hal_present(GFX_Canvas buffer, int32 mx, int32 my)
{
    for (int i = 0; i < (buffer.px_stride * buffer.px_h); i++)
    {
        frame_buffer.px[i] = rgba_to_abgr(buffer.px[i]);
    }

    // gfx_fill_rect(frame_buffer, mx, my, 8, 16, COL(0xFF,0xFF, 0xFF, 0xFF));
    for (int y = 0; y < 16; y++){
        for (int x = 0; x < 8; x++) {
            frame_buffer.px[(my + y) * frame_buffer.px_stride + (mx + x)] = COL(0xFF,0xFF, 0xFF, 0xFF);
        }
    }

    __hal_present(
        frame_buffer.px,
        frame_buffer.px_w,
        frame_buffer.px_h,
        frame_buffer.px_stride
    );
}

uint32 hal_get_width()
{
    return frame_buffer.px_w;
}

uint32 hal_get_height()
{
    return frame_buffer.px_h;
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

extern int main(void);

void kernelMain(uint32 width, uint32 height) {
    // TODO: remove this malloc.
    // TODO: the only need for "frame_buffer" is to convert from rgba to argb.
    fb_px = malloc(width * height * sizeof(uint32));

    frame_buffer = (GFX_Canvas) {
        .px = fb_px,
        .px_stride = width,
        .px_w = width,
        .px_h = height,
    };

    (void)main();
}
