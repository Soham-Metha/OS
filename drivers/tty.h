/* tty.h */

#ifndef TTY_1
#define TTY_1

#include "terminal.h"
#define TTY_BUF_SIZE 128

typedef struct tty {
    Terminal term;
    uint8 buf[TTY_BUF_SIZE];
    uint8 head;
    uint8 tail;
    bool echo;
} tty;

void tty_init(struct tty* t, int width, int height, uint32 fg, uint32 bg);
void tty_push_key(tty* t, uint8 c);
uint8 tty_read_char(tty* t);
void tty_write_char(tty* t, char c);
void tty_flush(tty* t);

#endif
#ifdef IMPL_TTY_1

#define IMPL_TERMINAL_1
#include "terminal.h"

tty* active_tty = (void*)0;

void tty_init(struct tty* t, int width, int height, uint32 fg, uint32 bg)
{
    if (!t)
        return;

    terminal_init(&t->term, height, width, fg, bg);

    t->head    = 0;
    t->tail    = 0;
    t->echo    = true;

    active_tty = t;
    terminal_draw_cursor(&t->term);
    hal_present();
}

void tty_push_key(tty* t, uint8 c)
{
    t->buf[t->head] = c;
    t->head         = (t->head + 1) % TTY_BUF_SIZE;

    if (t->echo) {
        tty_write_char(t, c);
        tty_flush(t);
    }
}

uint8 tty_read_char(tty* t)
{
    if (t->head == t->tail)
        return -1;
    char c  = t->buf[t->tail];
    t->tail = (t->tail + 1) % TTY_BUF_SIZE;
    return c;
}

void tty_write_char(tty* t, char c)
{
    terminal_put_char(&t->term, c);
    if (c == '\n')
        tty_flush(t);
}

void tty_flush(tty* t)
{
    terminal_render(&t->term);
}

#else

extern tty* active_tty;

#endif
