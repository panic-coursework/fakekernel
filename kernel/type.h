#pragma once

#include <stdint.h>

#define NULL 0

typedef int8_t i8;
typedef uint8_t u8;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;

_Static_assert(sizeof(i64) == 8, "Size of i64 is not 8");

typedef u64 size_t;
