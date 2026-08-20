/*
 * types.h
 *  Copyright (C) 2026 Soham Metha
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef TYPES_1
#define TYPES_1

typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned int uint32;
typedef unsigned long long int uint64;
typedef signed char int8;
typedef signed short int int16;
typedef signed int int32;
typedef signed long long int int64;
typedef uint16 file_discriptor;
typedef uint32 uintPtr;

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wkeyword-macro"
#endif

#define bool _Bool
#define true 1
#define false 0
#define stdin 0
#define stdout 1
#define va_list __builtin_va_list
#define va_start __builtin_va_start
#define va_end __builtin_va_end
#define va_arg __builtin_va_arg

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#define public __attribute__((visibility("default")))
#define private static

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
