/* errors.h */

#ifndef ERRORS_1
#define ERRORS_1

typedef enum Error {
    ERR_BAD_IO,
    ERR_INVALID_SYSCALL,
    ERR_BAD_FILE_DESC,
    ERR_NO_INPUT_AVAILABLE,
    ERR_COUNT
} Error;

#endif
