/* kernel.h */

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wkeyword-macro"
#endif

#define bool _Bool
#define true 1
#define false 0

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#define public __attribute__((visibility("default")))
#define private static

typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned int uint32;
typedef unsigned long long int uint64;
typedef uint16 file_discriptor;

typedef enum Error {
    ERR_OK = 0,
    ERR_BAD_IO,
    ERR_INVALID_SYSCALL,
    ERR_BAD_FILE_DESC,
    ERR_COUNT
} Error;

#define reterr(x)  \
    do {           \
        errno = x; \
        return 0;  \
    } while (false)

extern Error errno;

typedef enum Syscall {
    SYS_READ,
    SYS_WRITE,
    SYS_COUNT,
} Syscall;

int __syscall_dispatch(Syscall s, uint64 a, uint64 b, uint64 c);
