#include "osapi.h"
#include <kernel/kernel.h>

public
void store(file_discriptor fd, uint8 c)
{
    __syscall_dispatch(SYS_WRITE, fd, c, 0);
}

public
Result8 load(file_discriptor fd)
{
    return __syscall_dispatch(SYS_READ, fd, 0, 0);
}

public
void p_yield()
{
    __syscall_dispatch(SYS_YIELD, 0, 0, 0);
}

public
void p_exit()
{
    __syscall_dispatch(SYS_EXIT, 0, 0, 0);
}
