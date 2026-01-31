/* tty.h */

#ifndef TTY_1
#define TTY_1

#define TTY_BUF_SIZE 1024
#include <common/result.h>

typedef struct tty {
    uint8 in_buf[TTY_BUF_SIZE];
    uint8 in_head;
    uint8 in_tail;
    uint8 out_buf[TTY_BUF_SIZE];
    uint8 out_head;
    uint8 out_tail;
    bool echo;
} tty;

void tty_init(struct tty* t);
void tty_push_key(tty* t, uint8 c);
Result8 tty_read_char(tty* t);
void tty_write_char(tty* t, char c);
uint64 tty_read_out(tty* t, uint8* buf, uint64 n);

#endif
#ifdef IMPL_TTY_1
#undef IMPL_TTY_1

#include <kernel/kernel.h>

void tty_init(struct tty* t)
{
    if (!t)
        return;

    t->in_head   = 0;
    t->in_tail   = 0;
    t->out_head  = 0;
    t->out_tail  = 0;
    t->echo      = true;

    k.active_tty = t;
}

void tty_push_key(tty* t, uint8 c)
{
    t->in_buf[t->in_head] = c;
    t->in_head            = (t->in_head + 1) % TTY_BUF_SIZE;

    if (t->echo) {
        tty_write_char(t, c);
    }
}

Result8 tty_read_char(tty* t)
{
    if (t->in_head == t->in_tail)
        return Err8(ERR_NO_INPUT_AVAILABLE);
    char c     = t->in_buf[t->in_tail];
    t->in_tail = (t->in_tail + 1) % TTY_BUF_SIZE;
    return Ok8(c);
}

void tty_write_char(tty* t, char c)
{
    t->out_buf[t->out_head] = c;
    t->out_head             = (t->out_head + 1) % TTY_BUF_SIZE;
}

uint64 tty_read_out(tty* t, uint8* buf, uint64 n)
{
    uint64 i = 0;
    while (i < n && t->out_tail != t->out_head) {
        buf[i++]    = t->out_buf[t->out_tail];
        t->out_tail = (t->out_tail + 1) % TTY_BUF_SIZE;
    }
    return i;
}

#endif
