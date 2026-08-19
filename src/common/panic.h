/*
 * panic.h
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

void printf(const char* fmt, ...);

#define PANIC(fmt, ...)                                                     \
    do {                                                                    \
        printf("PANIC: %s:%d: " fmt "\n", __FILE__, __LINE__, __VA_ARGS__); \
        while (1) { }                                                       \
    } while (0)

#define assert(x) (void)((x) || (printf("ASSERT FAILURE: %s:%d\n", __FILE__, __LINE__), 0))

#define log(fmt, ...) printf("LOG: %s:%d: " fmt "\n", __FILE__, __LINE__, __VA_ARGS__);

#define err(fmt, ...)          \
    do {                       \
        log(fmt, __VA_ARGS__); \
        goto ret_err;          \
    } while (0);

#define try(cond, fmt, ...)        \
    do {                           \
        if (!(cond)) {             \
            log(fmt, __VA_ARGS__); \
            goto ret_err;          \
        }                          \
    } while (0);
