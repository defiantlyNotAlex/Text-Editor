#ifndef SHORT_TYPES_H_
#define SHORT_TYPES_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <uchar.h>

#define countof(arr) (sizeof(arr) / sizeof(arr[0]))
#define foreach(arr, it) for (typeof(arr[0]) it = arr; it != arr + countof(arr); ++it)

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  f32;
typedef double f64;

typedef char     c8;
typedef char16_t c16;
typedef char32_t c32;

typedef uintptr_t intptr;

typedef unsigned char byte;
typedef ptrdiff_t isize;
typedef size_t    usize;


#define BYTE_MAX UINT8_MAX
#define BYTE_MIN 0

#define I8_MAX  INT8_MAX
#define I8_MIN  INT8_MIN
#define I16_MAX INT16_MAX
#define I16_MIN INT16_MIN
#define I32_MAX INT32_MAX
#define I32_MIN INT32_MIN
#define I64_MAX INT64_MAX
#define I64_MIN INT64_MIN

#define U8_MAX  UINT8_MAX
#define U8_MIN  0
#define U16_MAX UINT16_MAX
#define U16_MIN 0
#define U32_MAX UINT32_MAX
#define U32_MIN 0
#define U64_MAX UINT64_MAX
#define U64_MIN 0

#define USIZE_MAX SIZE_MAX
#define USIZE_MIN 0
#define ISIZE_MAX I64_MAX
#define ISIZE_MIN I64_MIN

#endif //SHORT_TYPES_H_