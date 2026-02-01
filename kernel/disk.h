/* disk.h */

#ifndef DISK_API_1
#define DISK_API_1

#include <drivers/tmpdisk.h>

typedef enum disk_type {
    DISK_RAMDISK,
} disk_type;

typedef struct BlockDevice {
    bool (*read)(void* dev, uint32 block, void* buf);
    bool (*write)(void* dev, uint32 block, const void* buf);
    uint32 blk_cnt;
    void* ctx;
} BlockDevice;

bool blk_read(BlockDevice* bd, uint16 blk, void* buf);
bool blk_write(BlockDevice* bd, uint16 blk, const void* buf);
void blk_dump(BlockDevice* bd, uint32 blk);
BlockDevice disk_init(disk_type dt, uint8* disk_data, uint32 blk_cnt);

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

BlockDevice disk_init(disk_type dt, uint8* disk_data, uint32 blk_cnt)
{
    RamDisk* rd_ctx = (RamDisk*)kmalloc(sizeof(RamDisk));
    rd_ctx->data    = disk_data;
    rd_ctx->blocks  = blk_cnt;

    switch (dt) {
    case DISK_RAMDISK:
        return (BlockDevice) {
            .read    = ramdisk_read,
            .write   = ramdisk_write,
            .blk_cnt = blk_cnt,
            .ctx     = rd_ctx,
        };

    default:
        break;
    }
    return (BlockDevice) { 0 };
}

#endif
