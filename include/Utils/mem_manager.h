#ifndef UTILS_MEM_MANAGER_1
#define UTILS_MEM_MANAGER_1
#include <Utils/string_view.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REGION_DEFAULT_CAPACITY 65536
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

typedef struct Region Region;
typedef struct Arena Arena;

struct Region {
    Region* next;
    size_t capacity;
    size_t size;
    char buffer[];
};

struct Arena {
    Region* first;
    Region* last;
};

void* region_alloc(Arena* arena, size_t size);
const char* arena_sv_to_cstr(Arena* arena, String_View str);
String_View arena_cstr_concat(Arena* arena, const char* a, const char* b);
void arena_clear(Arena* arena);
void arena_free(Arena* arena);

#endif
#ifdef MEM_MANAGER_IMPL
#undef MEM_MANAGER_IMPL

Region* region_create(size_t capacity)
{
    const size_t partSize = sizeof(Region) + capacity;
    Region* part          = (Region*)malloc(partSize);
    memset(part, 0, partSize);
    part->capacity = capacity;
    return part;
}

char* arena_insert_or_expand(Arena* arena, Region* cur, size_t size, size_t addr_offset_mask)
{
    // pointer to the next unused address in buffer
    uintptr_t next_addr         = (uintptr_t)(cur->buffer + cur->size);
    // pointer to the next unused aligned address in buffer
    uintptr_t next_aligned_addr = (next_addr + addr_offset_mask) & ~addr_offset_mask;

    // `next_addr + addr_offset_mask` ensures that the address moves into the next aligned address
    //  `& ~addr_offset_mask` discards the offset bits from the address obtained, to get the actual aligned address

    // actual size we'll have to allocate, after considering the padding bits
    size_t real_size            = (next_aligned_addr - next_addr) + size;

    if (cur->size + real_size <= cur->capacity) {
        memset((char*)next_addr, 0, real_size);
        cur->size += real_size;
        return (char*)next_addr;
    }

    if (cur->next) {
        return arena_insert_or_expand(arena, cur->next, size, addr_offset_mask);
    }

    // worst case scenario: none of the existing nodes in the LL have enough memory
    // in this case, we'll have to create a new node & allocate space inside that node

    size_t aligned_size = (size + addr_offset_mask) & ~addr_offset_mask;
    Region* part        = region_create(MAX(aligned_size, REGION_DEFAULT_CAPACITY));

    cur                 = arena->last;
    cur->next           = part;
    arena->last         = part;

    return arena_insert_or_expand(arena, cur->next, size, addr_offset_mask);
}

void* region_alloc_aligned(Arena* arena, size_t size, size_t alignment)
{
    // arena is empty
    if (arena->last == NULL) {
        assert(arena->first == NULL);

        Region* part = region_create(MAX(size, REGION_DEFAULT_CAPACITY));

        arena->last  = part;
        arena->first = part;
    }

    // fetch pointer to the next available unused memory
    if (size == 0) {
        return arena->last->buffer + arena->last->size;
    }

    // ensure that alignment is a power of 2
    assert((alignment & (alignment - 1)) == 0);

    return arena_insert_or_expand(arena, arena->first, size, alignment - 1);
}

void* region_alloc(Arena* arena, size_t size)
{
    // wrapper function for alignment
    return region_alloc_aligned(arena, size, sizeof(void*));
}

const char* arena_sv_to_cstr(Arena* arena, String_View str)
{
    char* cstr = (char*)region_alloc(arena, str.len + 1);
    memcpy(cstr, str.data, str.len);
    cstr[str.len] = '\0';
    return cstr;
}

String_View arena_cstr_concat(Arena* arena, const char* a, const char* b)
{
    const size_t aLen = strlen(a);
    const size_t bLen = strlen(b);
    char* buf         = (char*)region_alloc(arena, aLen + bLen);
    memcpy(buf, a, aLen);
    memcpy(buf + aLen, b, bLen);
    return (String_View) {
        .len  = aLen + bLen,
        .data = buf
    };
}

void arena_clear(Arena* arena)
{
    for (Region* part = arena->first; part != NULL; part = part->next) {
        part->size = 0;
    }
    arena->last = arena->first;
}

void arena_free(Arena* arena)
{
    for (Region *part = arena->first, *next = NULL; part != NULL; part = next) {
        next = part->next;
        free(part);
    }
}

#endif