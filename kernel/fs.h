/* fs.h */

#ifndef FS_1
#define FS_1

#include <common/result.h>
#include <common/types.h>

#define BLOCK_SIZE 512
#define PTRS_PER_BLOCK 256
#define INODES_PER_BLOCK 16
#define PTRS_PER_INODE 8

typedef uint8 block[BLOCK_SIZE];
typedef uint8 bootsector[BLOCK_SIZE - 12];     // size of super block = 12
typedef uint8 bitmap;
typedef struct BlockDevice BlockDevice;

typedef enum Inode_types {
    NOT_VALID = (0 << 0),
    VALID     = (1 << 0),
    FILE      = (0 << 1 | VALID),
    DIR       = (1 << 1 | VALID),
} Inode_types;

// 1 superblock | >=1 inode blocks | data block |
// superblock: metadata of file structure itself
// inodeblock: block containing inodes
// inode: header that describes the actual files

// super block should end in 0xAA55 since it would be helpful when creating the bootloader for native arch
typedef struct Super_Block {
    bootsector boot;
    uint16 reserved;
    uint16 blk_cnt;      // total amt of blocks on disk
    uint16 iblk_cnt;     // total amt of inode blocks on disk
    uint16 i_cnt;        // total amt of inodes on disk
    uint32 magic;        // magic: DE AD AA 55
} __attribute__((packed)) Super_Block;

typedef struct filename {
    char name[8];
    char ext[5];
} filename;

typedef struct Inode {
    uint8 valid;     // valid, file/dir
    uint16 size;     // 0 for a dir, else file size
    filename name;
    uint16 indirect;     // ID of block, where block pointed itself contains ptr.s to other blocks
    uint16 direct[PTRS_PER_INODE];
} __attribute__((packed)) Inode;

// in case of directory, 'indirect' can point to other inode blocks [dir contents]
// in case of file, 'indirect' should point to data blocks only [for now atleast]

typedef union fsblock {
    uint8 data[BLOCK_SIZE];
    uint16 pointers[PTRS_PER_BLOCK];
    Inode inodes[INODES_PER_BLOCK];
    Super_Block super;
} fsblock;

typedef struct filesystem {
    BlockDevice* bd;
    uint8* blkmap;
    Super_Block meta;
} filesystem;

ResultPtr inode_lookup(filesystem* fs, uintPtr idx);
ResultPtr inode_create(filesystem* fs, filename* nm, Inode_types filetype);

ResultPtr fs_format(BlockDevice* bd, bootsector* mbr);
void fs_show(filesystem* fs, bool showbm);

#endif
#ifdef IMPL_FS_1
#undef IMPL_FS_1

#define IMPL_DISK_API_1
#include "disk.h"
#include <common/errors.h>
#include <common/heap.h>

#define setbit_(b, p) (b | (1 << (p)))
#define unsetbit_(b, p) (b & ~(1 << (p)))
void setbit(uint8* bm, uint16 idx, bool val)
{
    uint16 byte = idx >> 3;     // idx / 8
    uint8 bit   = idx & 7;      // idx % 8

    if (val)
        bm[byte] = setbit_(bm[byte], bit);
    else
        bm[byte] = unsetbit_(bm[byte], bit);
}

#define getbit_(b, p) ((b & (1 << (p))) >> (p))
bool getbit(uint8* bm, uint16 idx)
{
    uint16 byte = idx >> 3;
    uint8 bit   = idx & 7;

    return (bm[byte] >> bit) & 1;
}

bitmap* bitmap_create(filesystem* fs, bool scan)
{
    if (!fs)
        return 0;

    uint16 blocks = fs->bd->blk_cnt;
    uint16 bytes  = (blocks + 7) >> 3;

    bitmap* bm    = (bitmap*)kmalloc(bytes);
    if (!bm)
        return 0;

    memset(bm, 0, bytes);

    if (!scan)
        return bm;

    uint16 index = 0;
    fsblock blk;

    for (uint16 b = 1; b <= fs->meta.iblk_cnt; b++) {
        if (!blk_read(fs->bd, b, &blk))
            return 0;

        for (uint16 i = 0; i < INODES_PER_BLOCK; i++) {
            if (blk.inodes[i].valid & 0x01)
                setbit(bm, index, true);
            index++;
        }
    }

    return bm;
}

ResultPtr inode_lookup(filesystem* fs, uintPtr idx)
{
    if (!fs)
        return ErrPtr(ERR_BAD_IO);

    uintPtr max = fs->meta.iblk_cnt * INODES_PER_BLOCK;
    if (idx >= max)
        return ErrPtr(13);

    uintPtr block_idx  = idx / INODES_PER_BLOCK;
    uintPtr inode_idx  = idx % INODES_PER_BLOCK;
    uintPtr disk_block = 1 + block_idx;     // superblock
    fsblock bl         = { 0 };

    if (!blk_read(fs->bd, disk_block, &bl.data))
        return ErrPtr(ERR_BAD_IO);

    Inode* ino = (Inode*)kmalloc(sizeof(Inode));
    memcpy(ino, &bl.inodes[inode_idx], sizeof(Inode));
    return OkPtr((uintPtr)ino);
}

const char* get_inode_type_name(Inode_types i)
{
    switch (i) {
    case NOT_VALID: return "INVALID INODE!";
    case FILE:      return "FILE";
    case DIR:       return "DIR ";
    default:        return "??? ";
    }
}

void fs_show(filesystem* fs, bool showbm)
{
    if (!fs)
        return;

    printf("\n  %d total blocks, 1 superblock and %d inode blocks containing %d inodes",
        fs->meta.blk_cnt, fs->meta.iblk_cnt, fs->meta.i_cnt);

    for (uintPtr inodeno = 0; inodeno < fs->meta.i_cnt; inodeno++) {
        ResultPtr rino = inode_lookup(fs, inodeno);
        if RESULT_ERR (rino) {
            printf("\ninode err: %d", RESULT_ERRNO(rino));
            break;
        }

        Inode* ino = (Inode*)RESULT_VAL(rino);
        if (!(ino->valid & VALID))
            continue;

        printf("\nValid Inode: %d [ size:%d bytes, type:%s, ", inodeno, ino->size, get_inode_type_name(ino->valid));
        if (inodeno == 0)
            printf("name: / ]");
        else if (ino->valid == DIR)
            printf("name: %s ]", ino->name.name);
        else
            printf("name: %s.%s ]", ino->name.name, ino->name.ext);
    }
    printf("\n");

    if (!showbm)
        return;

    printf("Bitmap:");

    for (uintPtr n = 0; n < fs->bd->blk_cnt; n++) {
        if (!(n % 8))
            printf("\n %d | ", n);
        if (getbit(fs->blkmap, n))
            printf("1 ");
        else
            printf("0 ");
    }
    printf("\n\n");
}

ResultPtr fs_format(BlockDevice* bd, bootsector* mbr)
{
    if (!bd)
        return ErrPtr(ERR_BAD_IO);

    uint16 blk_cnt  = bd->blk_cnt;
    uint16 iblk_cnt = (blk_cnt / 10);     // inode blocks should occupy 10% of the disk
    if (blk_cnt % 10)
        iblk_cnt++;

    Super_Block super = {
        .magic    = 0xDEADAA55,
        .i_cnt    = 1,
        .iblk_cnt = iblk_cnt,
        .blk_cnt  = blk_cnt,
        .reserved = 0,
    };

    if (mbr)
        memcpy(&super.boot, mbr, 500);
    else
        memset(&super.boot, 0, 500);

    if (!blk_write(bd, 0, &super))     // block 0 = super block
        return ErrPtr(ERR_BAD_IO);

    block idxblock = { 0 };
    Inode idx      = { .valid = DIR };

    memcpy(&idxblock, &idx, sizeof(idx));     // block containing 1 inode only

    if (!blk_write(bd, 1, &idxblock))     // block 1 is where the inodes start
        return ErrPtr(ERR_BAD_IO);

    Inode ino = { 0 };
    for (uint16 n = 0; n < (iblk_cnt - 1); n++) {
        if (!blk_write(bd, (n + 2), &ino))
            return ErrPtr(ERR_BAD_IO);
    }

    uint16 size    = sizeof(filesystem);
    filesystem* fs = (filesystem*)kmalloc(size);
    if (!fs)
        return ErrPtr(ERR_BAD_IO);
    memset(fs, 0, size);

    fs->bd = bd;
    memcpy(&fs->meta, &super, BLOCK_SIZE);

    bitmap* bm = bitmap_create(fs, false);
    size       = 1 + fs->meta.iblk_cnt;     // superblock

    for (uint16 n = 0; n < size; n++)
        setbit(bm, n, true);

    fs->blkmap = bm;

    return OkPtr((uintPtr)fs);
}

bool char_is_valid(char c)
{
    if ('a' <= c && c <= 'z')
        return true;
    if ('A' <= c && c <= 'Z')
        return true;
    if ('0' <= c && c <= '9')
        return true;
    if (c == '-' || c == '_')
        return true;

    return false;
}

bool filename_is_valid(filename* name, Inode_types filetype)
{
    if (!name || !filetype)
        return false;
    else if ((filetype == DIR) && (name->ext[0]))
        return false;

    for (char* p = name->name; *p != '\0'; p++)
        if (!char_is_valid(*p))
            return false;

    if (filetype == FILE)
        for (char* p = name->ext; *p != '\0'; p++)
            if (!char_is_valid(*p))
                return false;

    return true;
}

uintPtr inode_alloc(filesystem* fs)
{
    if (!fs)
        return 0;

    uintPtr idx  = fs->meta.i_cnt;
    ResultPtr rp = inode_lookup(fs, idx);
    if RESULT_ERR (rp)
        return 0;

    Inode* p = (Inode*)RESULT_VAL(rp);
    if ((p->valid & VALID)) {
        return 0;
    }

    fs->meta.i_cnt += 1;

    if (!blk_write(fs->bd, 0, &fs->meta))
        return 0;

    return idx;
}

uintPtr fs_save_inode(filesystem* fs, Inode* ino, uintPtr idx)
{
    if (!fs || !ino)
        return 0;

    uintPtr block_idx  = idx / INODES_PER_BLOCK;
    uintPtr inode_idx  = idx % INODES_PER_BLOCK;
    uintPtr disk_block = 1 + block_idx;     // superblock
    fsblock bl         = { 0 };

    if (!blk_read(fs->bd, disk_block, &bl.data))
        return 0;

    memcpy(&bl.inodes[inode_idx], ino, sizeof(Inode));

    if (!blk_write(fs->bd, disk_block, &bl.data))
        return 0;

    return disk_block;
}

ResultPtr inode_create(filesystem* fs, filename* nm, Inode_types filetype)
{
    if (!fs || !nm || !filetype)
        return ErrPtr(1);     // TODO: create better error codes

    if (!filename_is_valid(nm, filetype))
        return ErrPtr(2);

    uintPtr idx = inode_alloc(fs);
    if (!idx)
        return ErrPtr(3);

    Inode inode = {
        .valid = filetype,
        .size  = 0,
    };

    memcpy(&inode.name, nm, sizeof(filename));
    if (!fs_save_inode(fs, &inode, idx)) {
        return ErrPtr(4);
    }

    return OkPtr(idx);
}

#endif
