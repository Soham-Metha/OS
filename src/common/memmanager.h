/*
 * memmanager.h
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
#ifndef UTILS_MEM_MANAGER_1
#define UTILS_MEM_MANAGER_1

#include "errors.h"
#include "strings.h"
#include "types.h"

#define REGION_DEFAULT_CAPACITY 64 * 1024
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

typedef struct Region Region;
typedef struct Arena Arena;

struct Region {
    Region* next;
    uint64 capacity;
    uint64 size;
    char buffer[];
};

struct Arena {
    Region* first;
    Region* last;
};

ResultPtr region_alloc(Arena* arena, uint64 size);
const char* arena_sv_to_cstr(Arena* arena, String_View str);
String_View arena_cstr_concat(Arena* arena, const char* a, const char* b);
void arena_clear(Arena* arena);
void arena_free(Arena* arena);

#endif
#ifdef MEM_MANAGER_IMPL
#undef MEM_MANAGER_IMPL

void* malloc(uint64 size);
void* memset(void* buf, char c, uint32 n);
void* memcpy(void* dst, const void* src, uint32 n);

Region* unused = (Region*)0;

Region* region_create(uint64 capacity)
{
    if (unused != (Region*)0 && unused->capacity >= capacity) {
        Region* ret = unused;
        unused      = unused->next;
        ret->next   = (Region*)0;
        return ret;
    }

    const uint64 partSize = sizeof(Region) + capacity;
    Region* part          = (Region*)malloc(partSize);
    memset(part, 0, partSize);
    part->capacity = capacity;
    return part;
}

ResultPtr arena_insert_or_expand(Arena* arena, Region* cur, uint64 size, uint64 addr_offset_mask)
{
    uintPtr next_addr         = (uintPtr)(cur->buffer + cur->size);
    uintPtr next_aligned_addr = (next_addr + addr_offset_mask) & ~addr_offset_mask;
    uint64 real_size          = (next_aligned_addr - next_addr) + size;

    if (cur->size + real_size <= cur->capacity) {
        memset((char*)next_addr, 0, real_size);
        cur->size += real_size;
        return OkPtr(next_aligned_addr);
    }

    if (cur->next) {
        return arena_insert_or_expand(arena, cur->next, size, addr_offset_mask);
    }

    uint64 aligned_size = (size + addr_offset_mask) & ~addr_offset_mask;
    Region* part        = region_create(MAX(aligned_size, REGION_DEFAULT_CAPACITY));

    cur                 = arena->last;
    cur->next           = part;
    arena->last         = part;

    return arena_insert_or_expand(arena, cur->next, size, addr_offset_mask);
}

ResultPtr region_alloc_aligned(Arena* arena, uint64 size, uint64 alignment)
{
    if (arena->last == (Region*)0 && arena->first == (Region*)0) {
        Region* part = region_create(MAX(size, REGION_DEFAULT_CAPACITY));

        arena->last  = part;
        arena->first = part;
    }

    if (size == 0) {
        return OkPtr((uintPtr)(arena->last->buffer + arena->last->size));
    }

    try((alignment & (alignment - 1)) == 0, "alignment not a power of 2", "");

    return arena_insert_or_expand(arena, arena->first, size, alignment - 1);
ret_err:
    return ErrPtr(0);
}

ResultPtr region_alloc(Arena* arena, uint64 size)
{
    return region_alloc_aligned(arena, size, sizeof(void*));
}

const char* arena_sv_to_cstr(Arena* arena, String_View str)
{
    ResultPtr space = region_alloc(arena, str.len + 1);
    try(RESULT_OK(space), "out of space!", "");
    char* cstr = (char*)RESULT_VAL(space);
    memcpy(cstr, str.data, str.len);
    cstr[str.len] = '\0';
    return cstr;
ret_err:
    return "";
}

String_View arena_cstr_concat(Arena* arena, const char* a, const char* b)
{
    const uint64 aLen = strlen(a);
    const uint64 bLen = strlen(b);

    ResultPtr space   = region_alloc(arena, aLen + bLen);
    try(RESULT_OK(space), "out of space!", "");
    char* buf = (char*)RESULT_VAL(space);
    memcpy(buf, a, aLen);
    memcpy(buf + aLen, b, bLen);
    return (String_View) {
        .len  = aLen + bLen,
        .data = buf
    };
ret_err:
    return (String_View) { 0 };
}

void arena_clear(Arena* arena)
{
    for (Region* part = arena->first; part != (Region*)0; part = part->next) {
        part->size = 0;
    }
    arena->last = arena->first;
}

void arena_free(Arena* arena)
{
    for (Region *part = arena->first, *next = (Region*)0; part != (Region*)0; part = next) {
        next       = part->next;
        part->next = unused;
        unused     = part;
    }
}

#endif
