/**
 * @file univ_defs.h
 * @brief Universal definitions for the 16-bit Virtual Gameboy (GBVM) library.
 *
 * This header file contains various universal definitions used in the GBVM library.
 * It includes necessary standard C libraries and defines several constants and types.
 *

 * @authors Soham Metha, Omkar Jagtap
 * @date January 2025
 */

#pragma once

#define INTERNAL_VMCALLS_CAPACITY 1024
#define EXTERNAL_VMCALLS_CAPACITY 1024
#define LOAD_FACTOR_THRESHOLD 0.75
#define _XOPEN_SOURCE_EXTENDED
#define BINDINGS_CAPACITY 1024
#define PROGRAM_CAPACITY 1024
#define MEMORY_CAPACITY 65536
#define LABELS_CAPACITY 1024
#define MAX_INCLUDE_LEVEL 10
#define STACK_CAPACITY 1024
#define FILE_MAGIC 0x484f53
#define FILE_VERSION 0x4D41
#define COMMENT_SYMBOL ';'
#define PREP_SYMBOL '%'

#include <assert.h>
#include "../../../../common/types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define PRIu64 "lu"
#define PRIi64 "li"
#define PRId64 "ld"
#define PRIX64 "lX"

typedef uint8 Byte;
typedef uint16 Word;
typedef uint32 DoubleWord;

typedef uint64 DataEntry;
typedef uint64 InstAddr;
typedef uint64 MemoryAddr;
typedef uint64 StackAddr;
typedef uint64 u64;
typedef int64 i64;
typedef double f64;

typedef union {
    uint64 u64;
    int64 i64;
    double f64;
    void* ptr;
} QuadWord;

/**
 * @brief Retrieves the next command line argument.
 *
 * This function retrieves the next command line argument from the argument list
 * and updates the argument count and argument list pointers to only contain the
 * remaining pointers.
 *
 * @param argc A pointer to the argument count.
 * @param argv A pointer to the argument list.
 * @return The next command line argument.
 */
char* getNextCmdLineArg(int* argc, char*** argv);

typedef struct String String;
typedef struct Region Region;

bool doesFileExist(const char* filePath);

String appendToPath(Region* arena, String base, String filePath);

QuadWord quadwordFromU64(uint64_t u64);

QuadWord quadwordFromI64(int64_t i64);

QuadWord quadwordFromF64(double f64);

QuadWord quadwordFromPtr(void* ptr);