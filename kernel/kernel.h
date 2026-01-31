/* kernel.h */
#ifndef KERNEL_1
#define KERNEL_1

#include <common/result.h>
#include <common/types.h>
#include <drivers/tty.h>

typedef enum Syscall {
    SYS_READ,
    SYS_WRITE,
    SYS_YIELD,
    SYS_EXIT,
    SYS_COUNT,
} Syscall;

typedef struct Kernel {
    tty* active_tty;
} Kernel;

extern Kernel k;

Result8 __syscall_dispatch(Syscall s, uint64 a, uint64 b, uint64 c);

#endif
