#include "irq.h"

#include "memlayout.h"
#include "mm.h"
#include "mmdefs.h"
#include "panic.h"
#include "printf.h"
#include "riscv.h"
#include "trampoline.h"
#include "type.h"

void user_trap () {
  printk("user trap\n");
  dump_csr_s();
  while (true) continue;
}

void irq_init () {
  trapframe.user_trap = (u64) user_trap;
  u64 stvec = (u64)_user_to_kernel - (u64)&trampoline + TRAMPOLINE;
  csrw("stvec", stvec);
  u64 sie = (1 << 9) | (1 << 1);
  csrw("sie", sie);
}


__attribute__((noreturn)) void return_to_user (page_table_t table) {
  __attribute__((noreturn)) void (*trampoline_kernel_to_user)(satp) =
    (void *) ((u64)_kernel_to_user - (u64)&trampoline + TRAMPOLINE);
  trampoline_kernel_to_user(satp_from_table(table));
}
