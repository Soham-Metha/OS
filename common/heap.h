#ifndef KMALLOC_1
#define KMALLOC_1

#include "types.h"
#define KHEAP_SIZE (8 * 1024 * 1024)

void* kmalloc(uint64 size);

#endif
#ifdef IMPL_KMALLOC_1
#undef IMPL_KMALLOC_1

static uint8 kheap[KHEAP_SIZE];
static uint64 heap_offset = 0;

void* kmalloc(uint64 size)
{
    if (size == 0)
        return 0;

    size = (size + 7) & ~7;

    if (heap_offset + size > KHEAP_SIZE)
        return 0;

    void* ptr = &kheap[heap_offset];
    heap_offset += size;
    return ptr;
}

#endif
