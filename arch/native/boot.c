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

static inline void pic_unmask(uint8 irq)
{
    uint16 port;
    uint8 value;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }

    value = inb(port) & ~(1 << irq);
    outb(port, value);
}

void pic_send_eoi(uint8 irq)
{
    if (irq >= 8)
        outb(PIC2_COMMAND, PIC_EOI);

    outb(PIC1_COMMAND, PIC_EOI);
}

void ps2_write(uint8 val)
{
    while (inb(PS2_CMD) & 0x02);
    outb(PS2_DATA, val);
}

void ps2_cmd(uint8 val)
{
    while (inb(PS2_CMD) & 0x02);
    outb(PS2_CMD, val);
}

uint8 mouse_read()
{
    while (!(inb(PS2_CMD) & 0x01));
    return inb(PS2_DATA);
}

void mouse_write_ack(uint8 val)
{
    ps2_cmd(0xD4);
    ps2_write(val);
    (void)mouse_read();     // ACK
}

// timer
void irq0_handler_c(void)
{
    kernel_irq(IRQ_TIMER, (IRQ_Data) { .timer_freq = HZ });
    pic_send_eoi(0);
}

// keyboard
void irq1_handler_c(void)
{
    uint8 scan_code = inb(0x60);
    kernel_irq(IRQ_KEYBOARD, (IRQ_Data) { .keycode = scan_code });
    pic_send_eoi(1);
}

// https://wiki.osdev.org/Mouse_Input
void irq12_handler_c(void)
{
    static int mouse_cycle = 0;
    static uint8 mouse_packet[3];
    uint8 data = inb(0x60);

    // Sync: bit 3 must be set on first byte
    if (mouse_cycle == 0 && !(data & 0x08)) {
        pic_send_eoi(12);
        return;
    }

    mouse_packet[mouse_cycle++] = data;

    if (mouse_cycle == 3) {
        mouse_cycle = 0;

        // Discard overflow packets
        if (mouse_packet[0] & 0xC0) {
            pic_send_eoi(12);
            return;
        }

        Mouse_Movement mm;
        mm.left   = mouse_packet[0] & 0x01;
        mm.right  = mouse_packet[0] & 0x02;
        mm.middle = mouse_packet[0] & 0x04;

        mm.dx     = mouse_packet[1];
        mm.dy     = mouse_packet[2];

        if (mouse_packet[0] & 0x10)
            mm.dx |= 0xFFFFFF00;
        if (mouse_packet[0] & 0x20)
            mm.dy |= 0xFFFFFF00;
        mm.dy = -mm.dy;     // PS/2 Y axis inversion

        kernel_irq(IRQ_MOUSE, (IRQ_Data) { .mouse_movement = mm });
    }

    pic_send_eoi(12);
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
    vectors[vector]        = true;
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
    }

    setIdtEntry(32, (void*)(uint32)isr_stub_32, 0x8E);
    setIdtEntry(33, (void*)(uint32)isr_stub_33, 0x8E);
    setIdtEntry(44, (void*)(uint32)isr_stub_44, 0x8E);

    pic_remap(0x20, 0x28);

    /* mask everything */
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);

    ps2_cmd(0xA8); /* Enable PS/2 mouse (aux device) */
    ps2_cmd(0x20); /* Read PS/2 controller config byte */
    uint8 config = mouse_read();

    ps2_cmd(0x60);         /* Write config byte back */
    ps2_write(config | 2); /* Enable mouse IRQ (IRQ12) */

    mouse_write_ack(0xF6); /* Set mouse defaults */
    mouse_write_ack(0xF4); /* Enable mouse data reporting */

    pic_unmask(0);      // timer
    pic_unmask(1);      // keyboard
    pic_unmask(2);      // cascade
    pic_unmask(12);     // mouse

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

void switch_to(void (*func)(void))
{
    func();
}
