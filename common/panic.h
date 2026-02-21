/* panic.h */

void printf(const char* fmt, ...);

#define PANIC(fmt, ...)                                                     \
    do {                                                                    \
        printf("PANIC: %s:%d: " fmt "\n", __FILE__, __LINE__, __VA_ARGS__); \
        while (1) { }                                                       \
    } while (0)

#define assert(x) (void)((x) || (printf("ASSERT FAILURE: %s:%d\n", __FILE__, __LINE__), 0))

#define log(fmt, ...) printf("LOG: %s:%d: " fmt "\n", __FILE__, __LINE__, __VA_ARGS__);

#define err(fmt, ...)          \
    do {                       \
        log(fmt, __VA_ARGS__); \
        goto ret_err;          \
    } while (0);
