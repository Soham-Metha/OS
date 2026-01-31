/* io.h */
#ifndef USPACE_IO_1
#define USPACE_IO_1
#include <common/result.h>

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

Result8 getch()
{
    return load(1);
}

ResultPtr getline()
{
    static char buf[255];
    static int size = 0;

    Result8 r       = getch();
    if RESULT_OK (r) {
        buf[size++] = (char)RESULT_VAL(r);

        if ((char)RESULT_VAL(r) != '\n') {
            return ErrPtr(1);
        }

        buf[size] = '\0';
        size      = 0;
        return OkPtr((uintPtr)buf);
    }
    return ErrPtr(0);
}

#endif
