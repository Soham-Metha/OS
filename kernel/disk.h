/* disk.h */

#ifndef DISK_API_1
#define DISK_API_1

#include <common/heap.h>
#include <common/types.h>

#define BLOCK_SIZE 512

typedef enum disk_type {
    DISK_RAMDISK,
} disk_type;

typedef struct {
    uint8* data;
    uint32 blocks;
} RamDisk;

typedef struct BlockDevice {
    bool (*read)(void* dev, uint32 block, void* buf);
    bool (*write)(void* dev, uint32 block, const void* buf);
    uint32 blk_cnt;
    void* ctx;
} BlockDevice;

bool blk_read(BlockDevice* bd, uint16 blk, void* buf);
bool blk_write(BlockDevice* bd, uint16 blk, const void* buf);
void blk_dump(BlockDevice* bd, uint32 blk);
BlockDevice disk_init(disk_type dt, uint32 blk_cnt);

#endif
#ifdef IMPL_DISK_API_1
#undef IMPL_DISK_API_1

bool blk_read(BlockDevice* bd, uint16 blk, void* buf)
{
    return bd->read(bd->ctx, blk, buf);
}

bool blk_write(BlockDevice* bd, uint16 blk, const void* buf)
{
    return bd->write(bd->ctx, blk, buf);
}

void blk_dump(BlockDevice* bd, uint32 blk)
{
    uint8 buf[BLOCK_SIZE];
    bd->read(bd->ctx, blk, buf);

    for (uint32 i = 0; i < BLOCK_SIZE; i++) {
        if (i % 32 == 0)
            printf("\n");
        printf("%d ", buf[i]);
    }
    printf("\n");
}

bool ramdisk_read(void* dev, uint32 block, void* buf)
{
    RamDisk* rd = (RamDisk*)dev;
    if (block >= rd->blocks)
        return false;

    memcpy(buf, rd->data + (block * BLOCK_SIZE), BLOCK_SIZE);
    return true;
}

bool ramdisk_write(void* dev, uint32 block, const void* buf)
{
    RamDisk* rd = (RamDisk*)dev;
    if (block >= rd->blocks)
        return false;

    memcpy(rd->data + (block * BLOCK_SIZE), buf, BLOCK_SIZE);
    return true;
}

BlockDevice disk_init(disk_type dt, uint32 blk_cnt)
{
    switch (dt) {
    case DISK_RAMDISK:
        {
            uint8* disk_data = (uint8*)kmalloc(64 * BLOCK_SIZE);
            RamDisk* rd_ctx  = (RamDisk*)kmalloc(sizeof(RamDisk));
            rd_ctx->data     = disk_data;
            rd_ctx->blocks   = blk_cnt;
            return (BlockDevice) {
                .read    = ramdisk_read,
                .write   = ramdisk_write,
                .blk_cnt = blk_cnt,
                .ctx     = rd_ctx,
            };
        }

    default:
        break;
    }
    return (BlockDevice) { 0 };
}

#endif
