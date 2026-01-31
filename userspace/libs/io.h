/* io.h */
#ifndef USPACE_IO_1
#define USPACE_IO_1

void putch(char c);
void print_str(const char* str);
void print_int(int i);
void printf(const char* fmt, ...);

char getch();
const char* getline();

#endif
#ifdef IMPL_USPACE_IO_1
#undef IMPL_USPACE_IO_1

#include <osapi/osapi.h>

void putch(char c)
{
    store(1, c);
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
            }
        } else {
            putch(*fmt);
        }
        fmt += 1;
    }

end:
    va_end(vargs);
}

char getch()
{
    char c = (signed char)load(1);
    if (c == -1) {
        p_yield();
        return -1;
    }
    return c;
}

const char* getline()
{
    static char buf[255];
    static int size = 0;

    char c          = getch();
    if (c == -1) {
        return "";
    }
    buf[size++] = c;

    if (c != '\n') {
        return "";
    }

    buf[size] = '\0';
    size      = 0;
    return buf;
}

#endif
