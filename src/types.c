#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <float.h>

#if INTERFACE

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef intptr_t intptr;
typedef uintptr_t uintptr;

typedef size_t memory_index;

typedef float float32;
typedef double float64;

#define internal static
#define local_persist static
#define global_variable static

#endif