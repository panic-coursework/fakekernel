#pragma once

#include <stdint.h>
#include <stdbool.h>

#define NULL 0

typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;

_Static_assert(sizeof(i64) == 8, "Size of i64 is not 8");

typedef u64 size_t;
typedef u64 pid_t;

#define let __auto_type
#define __noreturn __attribute__((noreturn))

#ifdef __clang__
#define __mm __attribute__((noderef))
#define __phy __attribute__((noderef, address_space(1)))
#define __user __attribute__((noderef, address_space(2)))
#else
#define __mm
#define __phy
#define __user
#endif
