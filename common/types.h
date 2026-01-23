/* types.h */

#ifndef TYPES_1
#define TYPES_1

typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned int uint32;
typedef unsigned long long int uint64;
typedef uint16 file_discriptor;

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wkeyword-macro"
#endif

#define bool _Bool
#define true 1
#define false 0

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#define public __attribute__((visibility("default")))
#define private static

#endif
