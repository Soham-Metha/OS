#include "univ_fileops.h"
#include "univ_defs.h"
#include "univ_errors.h"

FILE* openFile(const char* filePath, const char* mode)
{
    FILE* f = fopen(filePath, mode);

    if (!f) {
        fileErrorDispWithExit("can't open file", filePath);
    }

    return f;
}

void closeFile(FILE* file, const char* filePath)
{
    if (file == NULL) {
        return;
    }

    if (fclose(file) != 0) {
        fileErrorDispWithExit("can't close file", filePath);
    }
}

long getFileSize(FILE* f, const char* filePath)
{
    if (f == NULL) {
        fileErrorDispWithExit("invalid file pointer", filePath);
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fileErrorDispWithExit("can't read from file", filePath);
    }

    long fileSize = ftell(f);

    if (fileSize < 0) {
        fileErrorDispWithExit("can't read from file", filePath);
    }

    rewind(f);

    return fileSize;
}

const char* convertStrToArenaCstr(Arena* arena, String str)
{
    char* cstr = region_alloc(arena, str.length + 1);
    memcpy(cstr, str.data, str.length);
    cstr[str.length] = '\0';
    return cstr;
}

int loadFileIntoRegionStr(Arena* arena, String filePath, String* content)
{
    const char* filePathCstr = convertStrToArenaCstr(arena, filePath);

    FILE* f                  = openFile(filePathCstr, "rb");

    long m                   = getFileSize(f, filePath.data);

    char* buffer             = region_alloc(arena, (size_t)m);
    if (buffer == NULL) {
        return -1;
    }

    size_t n = fread(buffer, 1, (size_t)m, f);
    if (ferror(f)) {
        return -1;
    }

    closeFile(f, filePath.data);

    if (content) {
        content->length = n;
        content->data   = buffer;
    }

    return 0;
}
