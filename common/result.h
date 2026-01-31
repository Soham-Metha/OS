#ifndef RESULT_1
#define RESULT_1

#include "types.h"

#define _RESULT_DECL(size)                                     \
    typedef struct {                                           \
        bool ok;                                               \
        union {                                                \
            uint##size value;                                  \
            int error;                                         \
        };                                                     \
    } Result##size;                                            \
                                                               \
    static inline Result##size Ok##size(uint##size value)      \
    {                                                          \
        return (Result##size) { .ok = true, .value = value };  \
    }                                                          \
                                                               \
    static inline Result##size Err##size(int error)            \
    {                                                          \
        return (Result##size) { .ok = false, .error = error }; \
    }

#define RESULT_OK(r) ((r).ok)
#define RESULT_ERR(r) (!(r).ok)
#define RESULT_VAL(r) ((r).value)
#define RESULT_ERRNO(r) ((r).error)

_RESULT_DECL(8)
_RESULT_DECL(Ptr)

#endif
