/* tty.h */

#ifndef TTY_1
#define TTY_1

#define TTY_BUF_SIZE 2048
#include <common/result.h>
#include <common/errors.h>

typedef struct tty {
    uint16 in_buf[TTY_BUF_SIZE];
    uint16 in_head;
    uint16 in_tail;
    uint16 out_buf[TTY_BUF_SIZE];
    uint16 out_head;
    uint16 out_tail;
    bool echo;
} tty;

void tty_init(struct tty* t);
void tty_push_key(tty* t, uint8 c);
Result8 tty_read_char(tty* t);
void tty_write_char(tty* t, char c);
Result8 tty_pop_char(tty* t);

#endif
#ifdef IMPL_TTY_1
#undef IMPL_TTY_1

void tty_init(struct tty* t)
{
    if (!t)
        return;

    t->in_head   = 0;
    t->in_tail   = 0;
    t->out_head  = 0;
    t->out_tail  = 0;
    t->echo      = true;
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

Result8 tty_pop_char(tty* t)
{
    if (t->out_head == t->out_tail)
        return Err8(ERR_NO_INPUT_AVAILABLE);
    char c      = t->out_buf[t->out_tail];
    t->out_tail = (t->out_tail + 1) % TTY_BUF_SIZE;
    return Ok8(c);
}

#endif
