#pragma once

#define NULL 0

typedef signed char i8;
typedef unsigned char u8;
typedef int i32;
typedef unsigned u32;
typedef long i64;
typedef unsigned long u64;

_Static_assert(sizeof(i64) == 8, "Size of i64 is not 8");

typedef u64 size_t;
