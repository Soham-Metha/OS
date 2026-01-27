/* io.h */
#ifndef USPACE_IO_1
#define USPACE_IO_1

void putch(char c);
void print_str(const char* str);

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
        store(1, str[i]);
    }
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
