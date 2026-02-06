#ifndef UTILS_MEM_MANAGER_1
#define UTILS_MEM_MANAGER_1

#include "heap.h"
// #include "strings.h"
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

void* region_alloc(Arena* arena, uint64 size);
// const char* arena_sv_to_cstr(Arena* arena, String_View str);
// String_View arena_cstr_concat(Arena* arena, const char* a, const char* b);
void arena_clear(Arena* arena);
void arena_free(Arena* arena);

#endif
#ifdef MEM_MANAGER_IMPL
#undef MEM_MANAGER_IMPL

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
    Region* part          = (Region*)kmalloc(partSize);
    // memset(part, 0, partSize);
    part->capacity        = capacity;
    return part;
}

char* arena_insert_or_expand(Arena* arena, Region* cur, uint64 size, uint64 addr_offset_mask)
{
    uintPtr next_addr         = (uintPtr)(cur->buffer + cur->size);
    uintPtr next_aligned_addr = (next_addr + addr_offset_mask) & ~addr_offset_mask;
    uint64 real_size          = (next_aligned_addr - next_addr) + size;

    if (cur->size + real_size <= cur->capacity) {
        // memset((char*)next_addr, 0, real_size);
        cur->size += real_size;
        return (char*)next_addr;
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

void* region_alloc_aligned(Arena* arena, uint64 size, uint64 alignment)
{
    if (arena->last == (Region*)0 && arena->first == (Region*)0) {
        Region* part = region_create(MAX(size, REGION_DEFAULT_CAPACITY));

        arena->last  = part;
        arena->first = part;
    }

    if (size == 0) {
        return arena->last->buffer + arena->last->size;
    }

    // assert((alignment & (alignment - 1)) == 0, "alignment not a power of 2");

    return arena_insert_or_expand(arena, arena->first, size, alignment - 1);
}

void* region_alloc(Arena* arena, uint64 size)
{
    return region_alloc_aligned(arena, size, sizeof(void*));
}

// const char* arena_sv_to_cstr(Arena* arena, String_View str)
// {
//     char* cstr = (char*)region_alloc(arena, str.len + 1);
//     memcpy(cstr, str.data, str.len);
//     cstr[str.len] = '\0';
//     return cstr;
// }

// String_View arena_cstr_concat(Arena* arena, const char* a, const char* b)
// {
//     const uint64 aLen = strlen(a);
//     const uint64 bLen = strlen(b);
//     char* buf         = (char*)region_alloc(arena, aLen + bLen);
//     memcpy(buf, a, aLen);
//     memcpy(buf + aLen, b, bLen);
//     return (String_View) {
//         .len  = aLen + bLen,
//         .data = buf
//     };
// }

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
