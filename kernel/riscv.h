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

#define csrr(name)        \
  ({                      \
    u64 _csr_value;       \
    __asm__ volatile (    \
      "csrr %0, " name    \
      : "=r" (_csr_value) \
    );                    \
    _csr_value;           \
  })

#define csrw(name, var) \
  __asm__(              \
    "csrw " name ", %0" \
    : : "r" (var)       \
  )

static inline struct status read_sstatus () {
  return (struct status) { .value = csrr("sstatus") };
};

static inline void write_sstatus (struct status status) {
  csrw("sstatus", status.value);
}


static inline struct interrupt_bitmap read_sie () {
  return (struct interrupt_bitmap) { .value = csrr("sie") };
}

static inline void write_sie (struct interrupt_bitmap sie) {
  csrw("sie", sie.value);
}

static inline struct interrupt_bitmap read_sip () {
  return (struct interrupt_bitmap) { .value = csrr("sip") };
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
  return (struct cause) { .value = csrr("scause") };
}

void dump_csr_s ();
void dump_csr_m ();
void dump_csr_all ();

#define REG_ZERO 0
#define REG_RA 1
#define REG_SP 2
#define REG_GP 3
#define REG_TP 4
#define REG_T0 5
#define REG_T1 6
#define REG_T2 7
#define REG_S0 8
#define REG_S1 9
#define REG_A0 10
#define REG_A1 11
#define REG_A2 12
#define REG_A3 13
#define REG_A4 14
#define REG_A5 15
#define REG_A6 16
#define REG_A7 17
#define REG_S2 18
#define REG_S3 19
#define REG_S4 20
#define REG_S5 21
#define REG_S6 22
#define REG_S7 23
#define REG_S8 24
#define REG_S9 25
#define REG_S10 26
#define REG_S11 27
#define REG_T3 28
#define REG_T4 29
#define REG_T5 30
#define REG_T6 31
