#include "osapi.h"
#include <kernel/kernel.h>

public
void store(file_discriptor fd, uint8 c)
{
    __syscall_dispatch(SYS_WRITE, fd, c, 0);
}

public
uint8 load(file_discriptor fd)
{
    return (uint8)__syscall_dispatch(SYS_READ, fd, 0, 0);
}
