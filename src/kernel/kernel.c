/*
 * kernel.c
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
#define IMPL_ERRORS_1
#define IMPL_KMALLOC_1
#define IMPL_TTY_1
#include "kernel.h"
#include "heap.h"
#include "scheduler.h"
#include <common/errors.h>
#include <common/types.h>
#include <drivers/tty.h>

tty io_buffer = { .echo = true, .mode = TTY_CANONICAL };

private
Result8 _k_read(file_discriptor fd)
{
    // TODO: implement the fs
    // TODO: ownership of resources
    (void)fd;
    if (fd == stdout) {
        return tty_out_pop(&io_buffer);
    } else if (fd == stdin) {
        Result8 c = tty_in_pop(&io_buffer);
        if RESULT_ERR (c) {
            reschedule(TASK_BLOCKED);
        }
        return c;
    } else {
        return Err8(ERR_INVALID_SYSCALL);
    }
}

private
Result8 _k_write(file_discriptor fd, uint8 c)
{
    (void)fd;
    if (fd == stdin) {
        if (tty_in_push(&io_buffer, c))
            return Ok8(0);
        else
            return Err8(0);
    } else if (fd == stdout) {
        if (tty_out_push(&io_buffer, c))
            return Ok8(0);
        else
            return Err8(0);
    } else {
        return Err8(ERR_INVALID_SYSCALL);
    }
}

Result8 __syscall_dispatch(Syscall s, uint64 a, uint64 b, uint64 c)
{
    switch (s) {
    case SYS_READ:
        (void)b;
        (void)c;
        return _k_read(a);
    case SYS_WRITE:
        (void)c;
        return _k_write(a, b);
    case SYS_YIELD:
        (void)a;
        (void)b;
        (void)c;
        reschedule(TASK_WAITING);
        return Ok8(0);
    case SYS_EXIT:
        (void)a;
        (void)b;
        (void)c;
        reschedule(TASK_EXITED);
        return Ok8(0);
    case SYS_COUNT:
    default:
        return Err8(ERR_INVALID_SYSCALL);
    }
}
