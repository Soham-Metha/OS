/*
 * io.h
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
#ifndef USPACE_IO_1
#define USPACE_IO_1
#include <common/errors.h>
#include <common/types.h>

void putch(char c);
void print_str(const char* str);
void print_int(int i);
void printf(const char* fmt, ...);
Result8 getch();
ResultPtr getline();

#endif
#ifdef IMPL_USPACE_IO_1
#undef IMPL_USPACE_IO_1

#include <osapi/osapi.h>

void putch(char c)
{
    store(stdout, c);
}

void print_str(const char* str)
{
    for (int i = 0; str[i] != '\0'; i++) {
        putch(str[i]);
    }
}

void print_int(int i)
{
    unsigned magnitude = i;
    if (i < 0) {
        putch('-');
        magnitude = -magnitude;
    }

    unsigned divisor = 1;
    while (magnitude / divisor > 9)
        divisor *= 10;

    while (divisor > 0) {
        putch('0' + magnitude / divisor);
        magnitude %= divisor;
        divisor /= 10;
    }
}

void printf(const char* fmt, ...)
{
    va_list vargs;
    va_start(vargs, fmt);
    while (*fmt) {
        if (*fmt == '%') {
            fmt += 1;
            switch (*fmt) {
            case '\0':
                putch('%');
                goto end;
            case '%':
                putch('%');
                break;
            case 's':
                print_str(va_arg(vargs, const char*));
                break;
            case 'd':
                print_int(va_arg(vargs, int));
                break;
            case 'c':
                putch(va_arg(vargs, int));
                break;
            default:
                putch('%');
                putch(*fmt);
                break;
            }
        } else {
            putch(*fmt);
        }
        fmt += 1;
    }

end:
    va_end(vargs);
}

Result8 getch()
{
    return load(stdin);
}

ResultPtr getline()
{
    static char buf[255];
    static int size = 0;

    Result8 r       = getch();
    if RESULT_OK (r) {
        buf[size++] = (char)RESULT_VAL(r);

        if ((char)RESULT_VAL(r) != '\n') {
            return ErrPtr(ERR_NO_INPUT_AVAILABLE);
        }

        buf[size - 1] = '\0';     // replace '\n' with '\0'
        size          = 0;
        return OkPtr((uintPtr)buf);
    }
    return ErrPtr(ERR_NO_INPUT_AVAILABLE);
}

#endif
