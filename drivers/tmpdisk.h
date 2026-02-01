/* tmpdisk.h */

#include <common/heap.h>
#include <common/types.h>

#define BLOCK_SIZE 512

typedef struct {
    uint8* data;
    uint32 blocks;
} RamDisk;

bool ramdisk_read(void* dev, uint32 block, void* buf)
{
    RamDisk* rd = (RamDisk*)dev;
    if (block >= rd->blocks) return false;

    memcpy(buf, rd->data + (block * BLOCK_SIZE), BLOCK_SIZE);
    return true;
}

bool ramdisk_write(void* dev, uint32 block, const void* buf)
{
    RamDisk* rd = (RamDisk*)dev;
    if (block >= rd->blocks) return false;

    memcpy(rd->data + (block * BLOCK_SIZE), buf, BLOCK_SIZE);
    return true;
}
