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

struct cell {
    char ch;
    uint32 fg;
    uint32 bg;
};

typedef struct Terminal {
    struct cell buffer[65535];
    int cols;
    int rows;
    int cursor_x;
    int cursor_y;
    uint32 fg;
    uint32 bg;
} Terminal;

typedef struct tty {
    Terminal term;
    uint8 buf[TTY_BUF_SIZE];
    uint8 head;
    uint8 tail;
    bool echo;
} tty;

extern tty* active_tty;

void kernel_irq(Interrupt i, uint64 data);

void tty_init(struct tty* t, int width, int height, uint32 fg, uint32 bg);
void tty_push_key(tty* t, uint8 c);
uint8 tty_read_char(tty* t);
void tty_write_char(tty* t, char c);
void tty_flush(tty* t);

#define IDX(t, x, y) (y * t->cols + x)

void terminal_put_char(Terminal* t, char c);
void terminal_render(Terminal* t);
void terminal_init(Terminal* t, int row, int col, uint32 fg, uint32 bg);
void terminal_clear(Terminal* t);
