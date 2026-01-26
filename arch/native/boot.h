#include "../hal.h"

#define PIC1 0x20
#define PIC2 0xA0
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)

#define PIC_EOI 0x20

extern void* isr_stub_table[];
extern void gdtFlush(uint32);
extern int main(void);
extern void kernel_tick(void);
extern void isr_stub_32(void);
extern void isr_stub_33(void);

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

// https://users.utcluj.ro/~baruch/sie/labor/PS2/Scan_Codes_Set_1.htm
char scan_code_ascii[128] = {
    0, 27, '1', '2', '3', '4', '5', '6',          // 0x00 - 0x07
    '7', '8', '9', '0', '-', '=', '\b', '\t',     // 0x08 - 0x0F
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',       // 0x10 - 0x17
    'o', 'p', '[', ']', '\n', 0, 'a', 's',        // 0x18 - 0x1F
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',       // 0x20 - 0x27
    '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',       // 0x28 - 0x2F
    'b', 'n', 'm', ',', '.', '/', 0, '*',         // 0x30 - 0x37
    0, ' ', 0, 0, 0, 0, 0, 0,                     // 0x38 - 0x3F
    0, 0, 0, 0, 0, 0, 0, '7',                     // 0x40 - 0x47
    '8', '9', '-', '4', '5', '6', '+', '1',       // 0x48 - 0x4F
    '2', '3', '0', '.', 0, 0, 0, 0,               // 0x50 - 0x57
    0, 0, 0, 0, 0, 0, 0, 0,                       // 0x58 - 0x5F
    0, 0, 0, 0, 0, 0, 0, 0,                       // 0x60 - 0x67
    0, 0, 0, 0, 0, 0, 0, 0,                       // 0x68 - 0x6F
    0, 0, 0, 0, 0, 0, 0, 0                        // 0x70 - 0x77
};

void exception_handler(uint32 vector, uint32 error_code);
