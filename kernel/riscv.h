#pragma once

#include <stdbool.h>

#include "type.h"

struct status {
  union {
    struct __attribute__((packed)) {
      u32 : 1;
      bool sie: 1;
      u32 : 1;
      bool mie: 1;
      u32 : 1;
      bool spie: 1;
      bool ube: 1;
      bool mpie: 1;
      bool spp: 1;
      u32 vs: 2;
      u32 mpp: 2;
      u32 fs: 2;
      u32 xs: 2;
      bool mprv: 1;
      bool sum: 1;
      bool mxr: 1;
      bool tvm: 1;
      bool rw: 1;
      bool tsr: 1;
      u32 : 9;
      u32 uxl: 2;
      u32 sxl: 2;
      bool sbe: 1;
      bool mbe: 1;
      u32 : 25;
      bool sd: 1;
    };
    u64 value;
  };
};
_Static_assert(sizeof(struct status) == 8, "");

#define csrr(name, var) \
  __asm__(              \
    "csrr %0, " name    \
    : "=r" (var)        \
  )

#define csrw(name, var) \
  __asm__(              \
    "csrw " name ", %0" \
    : : "r" (var)       \
  )

static inline struct status read_sstatus () {
  struct status s;
  csrr("sstatus", s.value);
  return s;
};

static inline void write_sstatus (struct status status) {
  csrw("sstatus", status.value);
}

void dump_csr_s ();
void dump_csr_m ();
void dump_csr_all ();
