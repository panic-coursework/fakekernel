#pragma once

#include <stdbool.h>

#include "memlayout.h"
#include "mm.h"
#include "panic.h"
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
  struct __attribute__((packed)) {
    u32 : 12;
    u32 vpn : 27;
  };
  u64 value;
  void __mm *ptr;
} sv39_va;

static inline void __phy *pa_from_ppn (u64 ppn) {
  return (void __phy *) (ppn << PAGE_INDEX_BITS);
}

typedef union {
  struct __attribute__((packed)) {
    u32 off: 12;
    u32 ppn0: 9;
    u32 ppn1: 9;
    u32 ppn2: 26;
  };
  struct __attribute__((packed)) {
    u32 : 12;
    u64 ppn : 44;
  };
  u64 value;
  void __phy *ptr;
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
  struct __attribute__((packed)) {
    u32 : 10;
    u64 ppn: 44;
    u32 : 10;
  };
  u64 value;
} sv39_pte;

static inline bool pte_is_leaf (u32 flags) {
  return (flags & (PTE_READ | PTE_WRITE | PTE_EXEC));
}
static inline bool pte_invalid_or_leaf (sv39_pte pte) {
  return !(pte.flags & PTE_VALID) || pte_is_leaf(pte.flags);
}
static inline page_table_t subtable_from_pte (sv39_pte pte) {
  return (page_table_t) VA(pa_from_ppn(pte.ppn));
}
static inline satp satp_from_table (page_table_t table) {
  sv39_pa pa = { .ptr = PA(table) };
  if (pa.off != 0) {
    panic("misaligned page table %p", table);
  }
  return (satp) {
    .mode = MM_SV39,
    .asid = 0,
    .ppn = pa.ppn,
  };
}
