/* os.h */

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
    ERR_BAD_FILE_DESC = 8,
    ERR_COUNT
} Error;

#define reterr(x)  \
    do {           \
        errno = x; \
        return 0;  \
    } while (false)

extern Error errno;

/* write 1 char */
public
bool store(file_discriptor fd, uint8 c);

/* read 1 char */
public
uint8 load(file_discriptor fd);
