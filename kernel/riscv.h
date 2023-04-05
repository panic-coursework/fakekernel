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
      u32 spp: 1;
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

struct interrupt_bitmap {
  union {
    struct __attribute__((packed)) {
      bool : 1;
      bool ssi : 1;
      bool : 1;
      bool msi : 1;
      bool : 1;
      bool sti : 1;
      bool : 1;
      bool mti : 1;
      bool : 1;
      bool sei : 1;
      bool : 1;
      bool mei : 1;
    };
    u64 value;
  };
};

struct cause {
  union {
    struct __attribute__((packed)) {
      u64 code : 63;
      bool interrupt : 1;
    };
    u64 value;
  };
};

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


static inline struct interrupt_bitmap read_sie () {
  struct interrupt_bitmap sie;
  csrr("sie", sie.value);
  return sie;
}

static inline void write_sie (struct interrupt_bitmap sie) {
  csrw("sie", sie.value);
}

static inline struct interrupt_bitmap read_sip () {
  struct interrupt_bitmap sip;
  csrr("sip", sip.value);
  return sip;
}

static inline void write_sip (struct interrupt_bitmap sip) {
  csrw("sip", sip.value);
}

static inline void enable_interrupts () {
  struct status s = read_sstatus();
  s.sie = true;
  write_sstatus(s);
}


#define CAUSE_SSI 1
#define CAUSE_MSI 3
#define CAUSE_STI 5
#define CAUSE_MTI 7
#define CAUSE_SEI 9
#define CAUSE_MEI 11

static inline struct cause read_scause () {
  struct cause scause;
  csrr("scause", scause.value);
  return scause;
}

void dump_csr_s ();
void dump_csr_m ();
void dump_csr_all ();
