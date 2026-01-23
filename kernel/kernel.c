#define IMPL_ERRORS_1

#include "kernel.h"
#include <common/errors.h>
#include <common/types.h>
#include <drivers/tty.h>

private
int _k_read(file_discriptor fd)
{
    (void)fd;
    return tty_read_char(active_tty);
}

private
int _k_write(file_discriptor fd, uint8 c)
{
    (void)fd;
    tty_write_char(active_tty, c);
    return ERR_OK;
}

int __syscall_dispatch(Syscall s, uint64 a, uint64 b, uint64 c)
{
    switch (s) {
    case SYS_READ:
        (void)b;
        (void)c;
        return _k_read(a);
    case SYS_WRITE:
        (void)c;
        return _k_write(a, b);
    case SYS_COUNT:
    default:
        return ERR_INVALID_SYSCALL;
    }
}
