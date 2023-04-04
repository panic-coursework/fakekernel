#pragma once

#include <stdbool.h>

#include "type.h"

#define MM_BARE 0
#define MM_SV39 8
#define MM_SV48 9
#define MM_SV57 10

typedef union {
  struct __attribute__((packed)) {
    u64 ppn: 44;
    u32 asid: 16;
    u32 mode: 4;
  };
  u64 value;
} satp;

typedef union {
  struct __attribute__((packed)) {
    u32 off: 12;
    u32 vpn0: 9;
    u32 vpn1: 9;
    u32 vpn2: 9;
  };
  u64 value;
} sv39_va;

typedef union {
  struct __attribute__((packed)) {
    u32 off: 12;
    u32 ppn0: 9;
    u32 ppn1: 9;
    u32 ppn2: 26;
  };
  u64 value;
} sv39_pa;

#define PTE_VALID  (1 << 0)
#define PTE_READ   (1 << 1)
#define PTE_WRITE  (1 << 2)
#define PTE_EXEC   (1 << 3)
#define PTE_USER   (1 << 4)
#define PTE_GLOBAL (1 << 5)
#define PTE_ACCESS (1 << 6)
#define PTE_DIRTY  (1 << 7)

typedef union {
  struct __attribute__((packed)) {
    u32 flags: 8;
    u32 rsw: 2;
    u32 ppn0: 9;
    u32 ppn1: 9;
    u32 ppn2: 26;
    u32 reserved: 10;
  };
  u64 value;
} sv39_pte;
