#include "univ_defs.h"

char* getNextCmdLineArg(int* argc, char*** argv)
{
    assert(*argc > 0);

    char* arg = **argv;

    *argc -= 1;
    *argv += 1;

    return arg;
}

String_View appendToPath(Arena* arena, String_View base, String_View filePath)
{
    const String_View sep   = STR("/");

    const uint64 resultSize = base.len + sep.len + filePath.len;
    char* result            = region_alloc(arena, resultSize);
    assert(result);

    char* append = result;

    memcpy(append, base.data, base.len);
    append += base.len;

    memcpy(append, sep.data, sep.len);
    append += sep.len;

    memcpy(append, filePath.data, filePath.len);
    append += filePath.len;

    return (String_View) {
        .len  = resultSize,
        .data = result,
    };
}

QuadWord quadwordFromU64(uint64 u64)
{
    return (QuadWord) { .u64 = u64 };
}

QuadWord quadwordFromI64(int64 i64)
{
    return (QuadWord) { .i64 = i64 };
}

QuadWord quadwordFromF64(double f64)
{
    return (QuadWord) { .f64 = f64 };
}

QuadWord quadwordFromPtr(void* ptr)
{
    return (QuadWord) { .ptr = ptr };
}
