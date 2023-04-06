#include "irq.h"

#include "memlayout.h"
#include "mm.h"
#include "mmdefs.h"
#include "panic.h"
#include "printf.h"
#include "riscv.h"
#include "sched.h"
#include "trap.h"
#include "type.h"
#include "uart.h"

static int handle_interrupt (struct cpu *regs, enum task_mode mode) {
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
    struct interrupt_bitmap sip = read_sip();
    sip.ssi = false;
    write_sip(sip);

#ifdef DEBUG_TIMER
    printk("timer interrupt\n");
#endif
    if (mode == MODE_SUPERVISOR) {
      current_task->kernel_frame = *regs;
    } else if (mode == MODE_USER) {
      current_task->user_frame = *regs;
    } else {
      panic("handle_interrupt: unknown mode");
    }
    current_task->mode = mode;
    schedule_next();

    panic("unreachable");
  }

  return 1;
}

struct cpu *kernel_trap (struct cpu *regs) {
  struct status sstatus = read_sstatus();
  if (!sstatus.spp) {
    dump_csr_s();
    panic("kernel_trap: called from user mode");
  }

  current_task->kernel_frame = *regs;
  csrr("sepc", current_task->kernel_frame.pc);

  if (handle_interrupt(regs, MODE_SUPERVISOR) == 0) return regs;
  dump_csr_s();
  panic("unable to handle kernel interrupt");
}

// FIXME
static page_table_t user_table;

__attribute__((noreturn)) void user_trap () {
  struct status sstatus = read_sstatus();
  if (sstatus.spp) {
    dump_csr_s();
    panic("user_trap: called from kernel mode");
  }

  csrw("stvec", (u64) kernelvec);

  if (handle_interrupt(&trapframe.task, MODE_USER) == 0) {
    return_to_user(user_table);
  }

  struct cause scause = read_scause();
  if (!scause.interrupt) {
    current_task->user_frame = trapframe.task;
    klongjmp(&current_task->kernel_frame);
  }

  dump_csr_s();
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


static inline void print_cpu_reg (struct cpu *cpu, int reg, const char *name) {
  printk("%s\t%p\n", name, cpu->registers[reg]);
}

void dump_cpu (struct cpu *cpu) {
  printk("pc\t%p\n", cpu->pc);
  print_cpu_reg(cpu, REG_A0, "a0");
  print_cpu_reg(cpu, REG_A1, "a1");
  print_cpu_reg(cpu, REG_A2, "a2");
  print_cpu_reg(cpu, REG_A3, "a3");
  print_cpu_reg(cpu, REG_A4, "a4");
  print_cpu_reg(cpu, REG_A5, "a5");
  print_cpu_reg(cpu, REG_A6, "a6");
  print_cpu_reg(cpu, REG_SP, "sp");
  print_cpu_reg(cpu, REG_RA, "ra");
}
