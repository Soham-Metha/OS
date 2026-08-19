/*
 * result.h
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
