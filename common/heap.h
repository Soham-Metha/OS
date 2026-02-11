#ifndef KMALLOC_1
#define KMALLOC_1

#include "types.h"
#define KHEAP_SIZE (8 * 1024 * 1024)

void* kmalloc(uint64 size);
void* memset(void* buf, char c, uint32 n);
void* memcpy(void* dst, const void* src, uint32 n);

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

void* memset(void* buf, char c, uint32 n)
{
    uint8* p = (uint8*)buf;
    while (n--)
        *p++ = c;
    return buf;
}

void* memcpy(void* dst, const void* src, uint32 n)
{
    uint8* d       = (uint8*)dst;
    const uint8* s = (const uint8*)src;
    while (n--)
        *d++ = *s++;
    return dst;
}

#endif
