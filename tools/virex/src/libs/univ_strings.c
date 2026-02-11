#include "univ_strings.h"

String convertCstrToStr(const char* cstr)
{
    return (String) {
        .length = strlen(cstr),
        .data = (cstr)
    };
}

static bool is_space(char x)
{
    return x == ' ' || x == '\t';
}

String ltrim(String s)
{
    uint64 i = 0;
    while (i < s.length && is_space(s.data[i])) {
        i++;
    }
    return (String) {
        .length = s.length - i,
        .data = s.data + i
    };
}

String rtrim(String s)
{
    uint64 i = s.length;
    while (i > 0 && is_space(s.data[i - 1])) {
        i--;
    }
    return (String) {
        .length = i,
        .data = s.data
    };
}

String trim(String s)
{
    return rtrim(ltrim(s));
}

String splitStrByChar(String* s, char c)
{
    uint64 i = 0;
    while (i < s->length && s->data[i] != c) {
        i++;
    }
    String res = {

        .length = i,
        .data = s->data
    };

    s->length -= i;
    s->data += i;

    if (i < s->length) {
        s->length -= 1;
        s->data += 1;
    }

    return res;
}

static bool is_digit(char x)
{
    return (x >= '0' && x <= '9');
}

uint64 convertStrToInt(String s)
{
    uint64 val = 0;
    uint64 i = 0;

    while (i < s.length && is_digit(s.data[i])) {
        val = val * 10 + s.data[i] - '0';
        i++;
    }
    return val;
}

void printString(String s)
{
    printf("%.*s\n", (int)(s.length), s.data);
}

bool compareStr(String a, String b)
{
    if (a.length != b.length) {
        return false;
    } else {
        return memcmp(a.data, b.data, a.length) == 0;
    }
}

bool getIndexOf(String str, char c, uint64* index)
{
    uint64 i = 0;
    while (i < str.length && str.data[i] != c) {
        i += 1;
    }

    if (i < str.length) {
        *index = i;
        return true;
    }
    return false;
}

bool startsWith(String str, String prefix)
{
    if (prefix.length <= str.length) {
        for (uint64 i = 0; i < prefix.length; ++i) {
            if (prefix.data[i] != str.data[i]) {
                return false;
            }
        }
        return true;
    }
    return false;
}

String splitStrByLen(String* str, uint64 n)
{
    if (n > str->length) {
        n = str->length;
    }

    String result = {
        .data = str->data,
        .length = n,
    };

    str->data += n;
    str->length -= n;

    return result;
}

String splitStrByCondition(String* str, bool (*predicate)(char x))
{
    uint64 i = 0;
    while (i < str->length && predicate(str->data[i])) {
        i += 1;
    }
    return splitStrByLen(str, i);
}

String splitStrByLenReversed(String* str, uint64 n)
{
    if (n > str->length) {
        n = str->length;
    }

    String result = {
        .data = str->data + str->length - n,
        .length = n
    };

    str->length -= n;

    return result;
}

bool endsWith(String str, String expectedSuffix)
{
    if (expectedSuffix.length <= str.length) {
        String actualSuffix = {
            .data = str.data + str.length - expectedSuffix.length,
            .length = expectedSuffix.length
        };

        return compareStr(expectedSuffix, actualSuffix);
    }

    return false;
}