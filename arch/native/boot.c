#include "boot.h"
#include <kernel/interrupt.h>

IDT_Entry idt[256];
GDT_Entry gdtEntries[5];
static bool vectors[256];

static uint32* fb;
static uint32 pitch;
static uint32 width;
static uint32 height;

static inline uint8 inb(uint16 port)
{
    uint8 ret;
    __asm__ volatile("inb %1, %0"
        : "=a"(ret)
        : "Nd"(port));
    return ret;
}

static inline void outb(uint16 port, uint8 value)
{
    __asm__ volatile("outb %0, %1"
        :
        : "a"(value), "Nd"(port));
}

// https://wiki.osdev.org/8259_PIC
void pic_remap(uint8 offset1, uint8 offset2)
{
    /* initialization sequence */
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);

    /* vector offsets */
    outb(PIC1_DATA, offset1);
    outb(PIC2_DATA, offset2);

    /* tell Master PIC there is a slave at IRQ2 (0000 0100) */
    outb(PIC1_DATA, 0x04);
    /* tell Slave PIC its cascade identity (0000 0010) */
    outb(PIC2_DATA, 0x02);

    /* set 8086/88 mode */
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    outb(PIC1_DATA, 0);
    outb(PIC2_DATA, 0);
}

void pic_send_eoi(uint8 irq)
{
    if (irq >= 8)
        outb(PIC2_COMMAND, PIC_EOI);

    outb(PIC1_COMMAND, PIC_EOI);
}

// timer
void irq0_handler_c(void)
{
    kernel_tick();
    pic_send_eoi(0);
}

// keyboard
void irq1_handler_c(void)
{
    uint8 scan_code = inb(0x60);
    if (scan_code < 128)
        kernel_irq(IRQ_KEYBOARD, scan_code_ascii[scan_code]);
    pic_send_eoi(1);
}

void exception_handler(uint32 vector, uint32 error_code)
{
    (void)vector;
    (void)error_code;
    __asm__ volatile("cli; hlt");     // !! hangs the computer
}

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

void setIdtEntry(uint8 vector, void* isr, uint8 flags)
{
    idt[vector].isr_low    = (uint32)isr & 0xFFFF;
    idt[vector].kernel_cs  = 0x08;
    idt[vector].attributes = flags;
    idt[vector].isr_high   = (uint32)isr >> 16;
    idt[vector].reserved   = 0;
}

extern void kernelMain(uint32 magic, struct multiboot_info* mbi)
{
    Descriptor_Ptr gdtPtr, idtr;
    (void)magic;
    gdtPtr.limit = (sizeof(GDT_Entry) * 5) - 1;
    gdtPtr.base  = (uint32)&gdtEntries;

    setGdtEntry(0, 0, 0x00000000, 0x00, 0x00);
    setGdtEntry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    setGdtEntry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    setGdtEntry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    setGdtEntry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);
    gdtFlush((uint32)&gdtPtr);

    idtr.limit = (sizeof(IDT_Entry) * 256) - 1;
    idtr.base  = (uint32)&idt;

    for (uint8 i = 0; i < 32; i++) {
        setIdtEntry(i, isr_stub_table[i], 0x8E);
        vectors[i] = true;
    }

    pic_remap(0x20, 0x28);

    /* mask everything */
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);

    /* unmask (IRQ0, IRQ1) */
    outb(0x21, 0xFc);

    setIdtEntry(32, (void*)(uint32)isr_stub_32, 0x8E);
    setIdtEntry(33, (void*)(uint32)isr_stub_33, 0x8E);
    vectors[32] = true;
    vectors[33] = true;

    __asm__ volatile("lidt %0" : : "m"(idtr));
    __asm__ volatile("sti");

    fb     = (uint32*)(uint32)mbi->framebuffer_addr;
    pitch  = mbi->framebuffer_pitch / 4;
    width  = mbi->framebuffer_width;
    height = mbi->framebuffer_height;

    (void)main();

    uint16 divisor = PIT_FREQ / HZ;

    outb(0x43, 0x36);               // Control: channel 0, lobyte/hibyte, mode 3
    outb(0x40, divisor & 0xFF);     // Data:    lo byte
    outb(0x40, divisor >> 8);       // Data     hi byte
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
            fb[y * pitch + x] = rgba >> 8;
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
