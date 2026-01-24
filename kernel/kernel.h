/* kernel.h */
#ifndef KERNEL_1
#define KERNEL_1

#include <common/types.h>
#include <userspace/services/wm.h>

typedef enum Syscall {
    SYS_READ,
    SYS_WRITE,
    SYS_COUNT,
} Syscall;

typedef struct Kernel {
    WindowManager wm;
    tty* active_tty;
} Kernel;

extern Kernel k;

int __syscall_dispatch(Syscall s, uint64 a, uint64 b, uint64 c);

#endif
