/*
 * kernel.h
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
#ifndef KERNEL_1
#define KERNEL_1

#include <common/types.h>
#include <drivers/tty.h>

typedef enum Syscall {
    SYS_READ,
    SYS_WRITE,
    // SYS_OPEN,
    // SYS_CLOSE,
    // SYS_CHMOD,
    SYS_YIELD,
    SYS_EXIT,
    SYS_COUNT,
} Syscall;

Result8 __syscall_dispatch(Syscall s, uint64 a, uint64 b, uint64 c);

#endif
