/* interrupt.h */

#define TTY_BUF_SIZE 128

typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned int uint32;
typedef unsigned long long int uint64;

typedef enum Interrupt {
    IRQ_KEYBOARD = 1,
    IRQ_COUNT,
} Interrupt;

void kernel_irq(Interrupt i, uint64 data);

void tty_push_key(uint8 c);
uint8 tty_read_char(void);
