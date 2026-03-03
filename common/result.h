#ifndef RESULT_1
#define RESULT_1

#include "types.h"

#define _RESULT_DECL(name, type)                               \
    typedef struct {                                           \
        bool ok;                                               \
        union {                                                \
            type value;                                        \
            int error;                                         \
        };                                                     \
    } Result##name;                                            \
                                                               \
    static inline Result##name Ok##name(type value)            \
    {                                                          \
        return (Result##name) { .ok = true, .value = value };  \
    }                                                          \
                                                               \
    static inline Result##name Err##name(int error)            \
    {                                                          \
        return (Result##name) { .ok = false, .error = error }; \
    }

#define RESULT_OK(r) ((r).ok)
#define RESULT_ERR(r) (!(r).ok)
#define RESULT_VAL(r) ((r).value)
#define RESULT_ERRNO(r) ((r).error)

_RESULT_DECL(8, uint8)
_RESULT_DECL(Ptr, uintPtr)

#endif
