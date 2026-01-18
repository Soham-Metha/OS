#ifndef UTILS_FILES_1
#define UTILS_FILES_1
#include <Utils/mem_manager.h>
#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

enum File_error {
    FILE_ERR_OK = 0,
    FILE_ERR_UNABLE_TO_OPEN,
    FILE_ERR_CANT_READ,
    FILE_ERR_CANT_REACH_END,
    FILE_ERR_CANT_READ_CONTENTS,
};

typedef enum File_error File_error;
typedef struct File_Content File_Content;

struct File_Content {
    String_View content;
    File_error error_code;
};

File_Content arena_file_open_into_sv(Arena* arena, const char* file_path);
String_View file_unwrap(File_Content f) ;

#endif
#ifdef UTILS_FILES_IMPL
#undef UTILS_FILES_IMPL

File_Content arena_file_open_into_sv(Arena* arena, const char* file_path)
{
    FILE* f;
    long file_size;
    File_Content out = {
        .content    = { .data = NULL, .len = 0 },
        .error_code = FILE_ERR_OK,
    };

    if (!(f = fopen(file_path, "r"))) {         out.error_code = FILE_ERR_UNABLE_TO_OPEN;
    } else if (fseek(f, 0, SEEK_END) != 0) {    out.error_code = FILE_ERR_CANT_READ;
    } else if ((file_size = ftell(f)) < 0) {    out.error_code = FILE_ERR_CANT_REACH_END;
    } else {
        char* buf      = (char*)region_alloc(arena, out.content.len + 1);
        buf[file_size] = '\0';

        rewind(f);
        out.content.len = file_size;
        if (out.content.len != fread(buf, 1, file_size, f))     out.error_code = FILE_ERR_CANT_READ_CONTENTS;
        else out.content.data = buf;
    }

    fclose(f);
    return out;
}

String_View file_unwrap(File_Content f) 
{
    assert(f.error_code == FILE_ERR_OK);
    return f.content;
}

#endif