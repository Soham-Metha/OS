#ifndef UTILS_STRING_VIEW_1
#define UTILS_STRING_VIEW_1
#include "types.h"

typedef struct String_View String_View;

struct String_View {
    uint64 len;
    const char* data;
};

#define STR(s) ((String_View) { .len = strlen(s), .data = s })
#define Str_Fmt(s) (int)s.len, s.data

int strlen(const char* str);
int sv_equals_cstr(String_View sv, const char* cstr);
char* strcpy(char* dst, const char* src);
String_View sv_trim_left(String_View s);
String_View sv_trim_right(String_View s);
String_View sv_trim(String_View s);
bool sv_compare(String_View a, String_View b);
bool sv_index_of(String_View str, char c, uint64* index);
String_View sv_split_by_len(String_View* str, uint64 n);
String_View sv_split_by_len_reversed(String_View* str, uint64 n);
String_View sv_split_by_delim(String_View* s, char c);
String_View sv_split_by_condition(String_View* str, bool (*predicate)(char x));
bool sv_starts_with(String_View str, String_View prefix);
bool sv_ends_with(String_View str, String_View expected);
uint64 sv_to_uint(String_View* s);
/*
#define SV_SCAN(sv, cnt, fmt, ...)              \
    do {                                        \
        int _used = 0;                          \
        int _ret  = sscanf((sv).data, fmt "%n", \
             __VA_ARGS__, &_used);              \
        assert(_ret == cnt);                    \
        assert((sv).len >= (uint64)_used);      \
        (sv).data += _used;                     \
        (sv).len -= _used;                      \
    } while (0)
*/
#endif

#ifdef STRING_VIEW_IMPL
#undef STRING_VIEW_IMPL

int strlen(const char* str)
{
    int i;
    for (i = 0; str[i] != '\0'; i += 1)
        ;
    return i;
}

char* strcpy(char* dst, const char* src)
{
    char* ret = dst;
    while ((*dst++ = *src++))
        ;
    return ret;
}

int sv_equals_cstr(String_View sv, const char* cstr)
{
    uint32 i = 0;

    while (i < sv.len && cstr[i]) {
        if (sv.data[i] != cstr[i])
            return 0;
        i++;
    }

    /* lengths must match exactly */
    return (i == sv.len && cstr[i] == '\0');
}

bool isspace(char c)
{
    return (c == ' ');
}

String_View sv_trim_left(String_View s)
{
    uint64 i = 0;
    while (i < s.len && isspace(s.data[i])) {
        i++;
    }
    return (String_View) { .len = s.len - i, .data = s.data + i };
}

String_View sv_trim_right(String_View s)
{
    uint64 i = s.len;
    while (i > 0 && isspace(s.data[i - 1])) {
        i--;
    }
    return (String_View) { .len = i, .data = s.data };
}

String_View sv_trim(String_View s)
{
    return sv_trim_right(sv_trim_left(s));
}

bool sv_compare(String_View a, String_View b)
{
    if (a.len != b.len) {
        return false;
    } else {
        for (uint64 i = 0; i < a.len; i++) {
            if (a.data[i] != b.data[i]) {
                return false;
            }
        }
        return true;
    }
}

bool sv_index_of(String_View str, char c, uint64* index)
{
    uint64 i = 0;
    while (i < str.len && str.data[i] != c) {
        i += 1;
    }

    if (i < str.len) {
        if (index)
            *index = i;
        return true;
    }
    return false;
}

String_View sv_split_by_len(String_View* str, uint64 n)
{
    if (n > str->len) {
        n = str->len;
    }

    String_View result = {
        .data = str->data,
        .len  = n,
    };

    str->data += n;
    str->len -= n;

    return result;
}

String_View sv_split_by_len_reversed(String_View* str, uint64 n)
{
    if (n > str->len) {
        n = str->len;
    }

    String_View result = {
        .data = str->data + str->len - n,
        .len  = n
    };

    str->len -= n;

    return result;
}

String_View sv_split_by_delim(String_View* s, char c)
{
    uint64 i = 0;
    while (i < s->len && s->data[i] != c) {
        i++;
    }

    String_View res = sv_split_by_len(s, i);

    if (s->len > 0 && s->data[0] == c) {     // discard the delim
        s->len -= 1;
        s->data += 1;
    }

    return res;
}

String_View sv_split_by_condition(String_View* str, bool (*predicate)(char x))
{
    uint64 i = 0;
    while (i < str->len && predicate(str->data[i])) {
        i += 1;
    }

    return sv_split_by_len(str, i);
}

bool sv_starts_with(String_View str, String_View prefix)
{
    if (prefix.len <= str.len) {
        for (uint64 i = 0; i < prefix.len; ++i) {
            if (prefix.data[i] != str.data[i]) {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool sv_ends_with(String_View str, String_View expected)
{
    if (expected.len <= str.len) {
        String_View actual = {
            .data = str.data + str.len - expected.len,
            .len  = expected.len
        };

        return sv_compare(expected, actual);
    }

    return false;
}

uint64 sv_to_uint(String_View* s)
{
    uint64 val = 0;
    while (s->len && s->data[0] >= '0' && s->data[0] <= '9') {
        val = val * 10 + s->data[0] - '0';
        sv_split_by_len(s, 1);
    }
    return val;
}

#endif
