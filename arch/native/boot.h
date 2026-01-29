#include "../hal.h"

#define PIC1 0x20
#define PIC2 0xA0
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)

#define PS2_DATA 0x60
#define PS2_CMD  0x64

#define PIC_EOI 0x20

#define PIT_FREQ 1193182
#define HZ 60

extern void* isr_stub_table[];
extern void isr_stub_32(void);
extern void isr_stub_33(void);
extern void isr_stub_44(void);
extern void gdtFlush(uint32);
extern int main(void);

typedef struct descriptor {
    uint16 limit_00_15;
    uint16 base_00_15;
    uint8 base_16_23;
    uint8 AccessByte;
    uint8 limit_16_19_flags;
    uint8 base_24_31;
} __attribute__((packed)) GDT_Entry;

typedef struct {
    uint16 isr_low;
    uint16 kernel_cs;
    uint8 reserved;
    uint8 attributes;
    uint16 isr_high;
} __attribute__((packed)) IDT_Entry;

typedef struct gdtPtr {
    uint16 limit;
    uint32 base;
} __attribute__((packed)) Descriptor_Ptr;

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

void exception_handler(uint32 vector, uint32 error_code);
