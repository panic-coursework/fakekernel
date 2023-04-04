#include "mm.h"

#include "memlayout.h"
#include "mmdefs.h"
#include "type.h"

static inline void set_satp (satp value) {
  __asm__(
    "csrw satp, %0\n\t"
    "sfence.vma"
    : : "r" (value.value)
  );
}

void establish_identical_mapping () {
  sv39_pte *table = (void *) KERN_ATP;
  sv39_pte value;
  value.flags = PTE_VALID | PTE_READ | PTE_WRITE | PTE_EXEC;
  value.rsw = 0;
  value.ppn0 = 0;
  value.ppn1 = 0;
  value.ppn2 = 0;
  value.reserved = 0;
  for (int i = 0; i < 1 << 9; ++i) {
    value.ppn2 = i;
    table[i] = value;
  }

  set_satp((satp) {
    .ppn = KERN_ATP >> 12,
    .asid = 0,
    .mode = MM_SV39,
  });
}
