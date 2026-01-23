/* errors.h */

#ifndef ERRORS_1
#define ERRORS_1

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

#endif
#ifdef IMPL_ERRORS_1

Error errno;

#else

extern Error errno;

#endif
