#include "irq.h"

#include "memlayout.h"
#include "mm.h"
#include "mmdefs.h"
#include "panic.h"
#include "printf.h"
#include "riscv.h"
#include "trap.h"
#include "type.h"
#include "uart.h"

static int handle_interrupt () {
  struct cause scause = read_scause();
  if (!scause.interrupt) return 1;

  if (scause.code == CAUSE_SEI) {
    u32 irq = *(u32 *)PLIC_SCLAIM(0);
    if (irq == UART0_IRQ) {
      uartintr();
      *(u32 *)PLIC_SCLAIM(0) = irq;
    } else if (irq) {
      panic("unknown irq");
    }
    return 0;
  }

  if (scause.code == CAUSE_SSI) {
#ifdef DEBUG_TIMER
    printk("timer interrupt\n");
#endif
    struct interrupt_bitmap sip = read_sip();
    sip.ssi = false;
    write_sip(sip);
    return 0;
  }

  return 1;
}

void kernel_trap () {
  if (handle_interrupt() == 0) return;
  dump_csr_s();
  panic("unable to handle kernel interrupt");
}

// FIXME
static page_table_t user_table;

__attribute__((noreturn)) void user_trap () {
  if (handle_interrupt() == 0) {
    return_to_user(user_table);
  }
  csrw("stvec", (u64) kernelvec);
  dump_csr_s();
  struct cause scause = read_scause();
  if (scause.interrupt == false && scause.code == 8) {
    printk("%d\n", trapframe.registers[10]);
  }
  panic("unable to handle user interrupt");
}

void irq_init () {
  printk("Initializing interrupts.\n");
  trapframe.user_trap = (u64) user_trap;
  u64 stvec = (u64) kernelvec;
  csrw("stvec", stvec);
  struct interrupt_bitmap sie = { .value = 0 };
  sie.ssi = true;
  sie.sei = true;
  csrw("sie", sie);

  *(u32 *)(PLIC + UART0_IRQ * 4) = 1;
  *(u32 *)PLIC_SENABLE(0) = (1 << UART0_IRQ);
  *(u32 *)PLIC_SPRIORITY(0) = 0;

  printk("Enabling interrupts.\n");
  enable_interrupts();
}


__attribute__((noreturn)) void return_to_user (page_table_t table) {
  // turn off interrupts
  struct status sstatus = read_sstatus();
  sstatus.spp = 0;
  sstatus.sie = false;
  sstatus.spie = true;
  write_sstatus(sstatus);

  u64 stvec = (u64)_user_to_kernel - (u64)&trampoline + TRAMPOLINE;
  csrw("stvec", stvec);

  user_table = table;

  __attribute__((noreturn)) void (*trampoline_kernel_to_user)(satp) =
    (void *) ((u64)_kernel_to_user - (u64)&trampoline + TRAMPOLINE);
  trampoline_kernel_to_user(satp_from_table(table));
}
