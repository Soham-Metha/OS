#include "../hal.h"
#include <common/types.h>

extern void gdtFlush(uint32);
extern int main(void);
extern void kernel_tick(void);

typedef struct descriptor {
    uint16 limit_00_15;
    uint16 base_00_15;
    uint8 base_16_23;
    uint8 AccessByte;
    uint8 limit_16_19_flags;
    uint8 base_24_31;
} __attribute__((packed)) Descriptor;

typedef struct gdtPtr {
    uint16 limit;
    uint32 base;
} __attribute__((packed)) GdtPtr;

struct multiboot_info {
    uint32 flags;         // indicates which fields are valid
    uint32 mem_lower;     // valid if flags[0] = 1
    uint32 mem_upper;
    uint32 boot_device;     // flags[1]
    uint32 cmdline;         // flags[2]
    uint32 mods_count;      // flags[3]
    uint32 mods_addr;
    uint32 syms[4];         // flags[4] & [5] (a.out or ELF)
    uint32 mmap_length;     // flags[6]
    uint32 mmap_addr;
    uint32 drives_length;     // flags[7]
    uint32 drives_addr;
    uint32 config_table;         // flags[8]
    uint32 boot_loader_name;     // flags[9]
    uint32 apm_table;            // flags[10]
    uint32 vbe_control_info;     // flags[11]
    uint32 vbe_mode_info;
    uint16 vbe_mode;
    uint16 vbe_interface_seg;
    uint16 vbe_interface_off;
    uint16 vbe_interface_len;
    // Framebuffer info (flags[12])
    uint64 framebuffer_addr;       // physical address of linear framebuffer
    uint32 framebuffer_pitch;      // bytes per scanline
    uint32 framebuffer_width;      // pixels
    uint32 framebuffer_height;     // pixels
    uint8 framebuffer_bpp;         // bits per pixel
    uint8 framebuffer_type;        // 0 = indexed, 1 = RGB, 2 = EGA text
    uint8 framebuffer_red_field_position;
    uint8 framebuffer_red_mask_size;
    uint8 framebuffer_green_field_position;
    uint8 framebuffer_green_mask_size;
    uint8 framebuffer_blue_field_position;
    uint8 framebuffer_blue_mask_size;
    uint8 reserved[6];     // padding to align struct
};

Descriptor gdtEntries[5];
GdtPtr gdtPtr;

static uint32* fb;
static uint32 pitch;
static uint32 width;
static uint32 height;

void setGdtEntry(uint32 num, uint32 base, uint32 limit, uint8 access, uint8 flags)
{
    gdtEntries[num].base_00_15        = base & 0xFFFF;
    gdtEntries[num].base_16_23        = (base >> 16) & 0xFF;
    gdtEntries[num].base_24_31        = (base >> 24) & 0xFF;

    gdtEntries[num].limit_00_15       = limit & 0xFFFF;
    gdtEntries[num].limit_16_19_flags = (limit >> 16) & 0x0F;

    gdtEntries[num].limit_16_19_flags |= flags & 0xF0;
    gdtEntries[num].AccessByte = access;
}

extern void kernelMain(uint32 magic, struct multiboot_info* mbi)
{
    (void)magic;
    gdtPtr.limit = (sizeof(Descriptor) * 5) - 1;
    gdtPtr.base  = (uint32)&gdtEntries;

    setGdtEntry(0, 0, 0, 0, 0);
    setGdtEntry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    setGdtEntry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    setGdtEntry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    setGdtEntry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);
    gdtFlush((uint32)&gdtPtr);

    fb     = (uint32*)(uint32)mbi->framebuffer_addr;
    pitch  = mbi->framebuffer_pitch / 4;
    width  = mbi->framebuffer_width;
    height = mbi->framebuffer_height;

    (void)main();
    (void)kernel_tick();
    // TODO: kernel tick should be periodically called
    // TODO: keyboard interrupt handler
}

void hal_put_pixel(int x, int y, uint32 rgba)
{
    if (x < 0 || y < 0 || x >= (int)width || y >= (int)height)
        return;
    fb[y * pitch + x] = rgba >> 8;
}

void hal_clear(uint32 rgba)
{
    (void)rgba;
    for (uint32 y = 0; y < height; y++)
        for (uint32 x = 0; x < width; x++)
            fb[y * pitch + x] =  rgba >> 8;
}

void hal_present(void)
{
    // nothing for now, pixels instantly render to screen
}

uint32 hal_get_width()
{
    return width;
}
uint32 hal_get_height()
{
    return height;
}
