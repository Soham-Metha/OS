/*
 * tty.h
 *  Copyright (C) 2026 Soham Metha
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef TTY_1
#define TTY_1

#define TTY_BUF_SIZE 2048
#define TTY_LINE_LIMIT 255
#include <common/errors.h>
#include <common/types.h>

typedef enum tty_mode {
    TTY_RAW,
    TTY_CANONICAL,
    TTY_CNT
} TTY_Mode;

typedef struct tty {
    uint8 in_buf[TTY_BUF_SIZE];
    uint8 out_buf[TTY_BUF_SIZE];
    uint8 line_buf[TTY_LINE_LIMIT];
    uint16 in_head;
    uint16 in_tail;
    uint16 out_head;
    uint16 out_tail;
    uint16 line_len;
    TTY_Mode mode;
    bool echo;
} tty;

void tty_init(struct tty* t);
bool tty_in_push(tty* t, uint8 c);
Result8 tty_in_pop(tty* t);
bool tty_out_push(tty* t, char c);
Result8 tty_out_pop(tty* t);

#endif
#ifdef IMPL_TTY_1
#undef IMPL_TTY_1

void tty_init(struct tty* t)
{
    if (!t)
        return;

    t->in_head  = 0;
    t->in_tail  = 0;
    t->out_head = 0;
    t->out_tail = 0;
    t->line_len = 0;
    t->mode     = TTY_CANONICAL;
    t->echo     = true;
}

#define EMPTY(head, tail) (head == tail)
#define FULL(head, tail, limit) (((head + 1) % limit) == tail)
#define AVAILABLE(head, tail, limit, needed)            \
    ((EMPTY(head, tail) && limit > needed)              \
        || (head < tail && (tail - head - 1) >= needed) \
        || (head > tail && (limit + tail - head - 1) >= needed))

// TODO: return err if full
#define PUSH(queue, head, tail, limit, ch) \
    {                                      \
        if FULL (head, tail, limit)        \
            return false;                  \
        queue[head] = ch;                  \
        head        = (head + 1) % limit;  \
    }

#define POP(queue, head, tail, limit, ch)        \
    {                                            \
        if EMPTY (head, tail)                    \
            return Err8(ERR_NO_INPUT_AVAILABLE); \
        ch   = queue[tail];                      \
        tail = (tail + 1) % limit;               \
    }

bool tty_in_push(tty* t, uint8 c)
{
    switch (t->mode) {
    case TTY_RAW:
        PUSH(t->in_buf, t->in_head, t->in_tail, TTY_BUF_SIZE, c)
        break;
    case TTY_CANONICAL:
        switch (c) {
        case '\b':
            if (t->line_len > 0)
                t->line_len -= 1;
            break;
        case '\n':
            if (!AVAILABLE(t->in_head, t->in_tail, TTY_BUF_SIZE, t->line_len + 1)) {
                return false;
            }

            for (uint16 i = 0; i < t->line_len; i++)
                PUSH(t->in_buf, t->in_head, t->in_tail, TTY_BUF_SIZE, t->line_buf[i])
            PUSH(t->in_buf, t->in_head, t->in_tail, TTY_BUF_SIZE, c)
            t->line_len = 0;
            break;
        default:
            PUSH(t->line_buf, t->line_len, 0, TTY_LINE_LIMIT, c)
            break;
        }
        break;
    case TTY_CNT:
        break;
    }

    if (t->echo) {
        tty_out_push(t, c);
    }
    return true;
}

bool tty_out_push(tty* t, char c)
{
    PUSH(t->out_buf, t->out_head, t->out_tail, TTY_BUF_SIZE, c);
    return true;
}

Result8 tty_in_pop(tty* t)
{
    char c;
    POP(t->in_buf, t->in_head, t->in_tail, TTY_BUF_SIZE, c)
    return Ok8(c);
}

Result8 tty_out_pop(tty* t)
{
    char c;
    POP(t->out_buf, t->out_head, t->out_tail, TTY_BUF_SIZE, c)
    return Ok8(c);
}

#endif
