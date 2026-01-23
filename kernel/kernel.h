/* kernel.h */

#include <common/types.h>

typedef enum Syscall {
    SYS_READ,
    SYS_WRITE,
    SYS_COUNT,
} Syscall;

int __syscall_dispatch(Syscall s, uint64 a, uint64 b, uint64 c);
