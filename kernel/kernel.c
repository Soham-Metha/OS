#define IMPL_ERRORS_1
#define IMPL_KMALLOC_1
#include "kernel.h"
#include "scheduler.h"
#include <common/errors.h>
#include <common/heap.h>
#include <common/types.h>
#include <drivers/tty.h>

Kernel k = { 0 };

private
int _k_read(file_discriptor fd)
{
    // TODO: implement the fs
    // TODO: ownership of resources
    (void)fd;
    char c = tty_read_char(k.active_tty);
    if (c == -1) {
        reschedule(TASK_BLOCKED);
        return -1;
    }
    return c;
}

private
int _k_write(file_discriptor fd, uint8 c)
{
    (void)fd;
    tty_write_char(k.active_tty, c);
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
    case SYS_YIELD:
        (void)a;
        (void)b;
        (void)c;
        reschedule(TASK_WAITING);
        return 0;
    case SYS_EXIT:
        (void)a;
        (void)b;
        (void)c;
        reschedule(TASK_EXITED);
        return 0;
    case SYS_COUNT:
    default:
        return ERR_INVALID_SYSCALL;
    }
}
