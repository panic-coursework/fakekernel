#include "irq.h"

#include "main.h"
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

bool may_page_fault;
struct cpu page_fault_handler;

static int handle_interrupt (struct cpu *regs, enum task_mode mode) {
  struct cause scause = read_scause();
  if (!scause.interrupt) return 1;

  if (scause.code == CAUSE_SEI) {
    u32 irq = *(u32 *)PLIC_SCLAIM(0);
    if (irq == UART0_IRQ) {
      uartintr();
      *(u32 *)PLIC_SCLAIM(0) = irq;
    } else if (irq) {
      panic("unknown irq %08x", irq);
    }
    return 0;
  }

  if (scause.code == CAUSE_SSI) {
    csrnc("sip", interrupt_bitmap, ssi);

#ifdef DEBUG_TIMER
    printk("timer interrupt\n");
#endif
    if (current_task == NULL) {
      schedule();
    }
    if (mode == MODE_SUPERVISOR) {
      current_task->kernel_frame = *regs;
    } else if (mode == MODE_USER) {
      current_task->user_frame = *regs;
    } else {
      panic("unknown mode %d", mode);
    }
    current_task->mode = mode;
    schedule_next();

    panic("unreachable");
  }

  return 1;
}

static struct cpu *kernel_trap (struct cpu *regs) {
  if (current_task != NULL) {
    current_task->kernel_frame = *regs;
    current_task->kernel_frame.pc = csrr("sepc");
  }

  if (handle_interrupt(regs, MODE_SUPERVISOR) == 0) {
    if (current_task != NULL) {
      return regs;
    }
    schedule();
  }

  struct cause scause = read_scause();
  // FIXME: magic number
  if (!scause.interrupt && (scause.code == 13 || scause.code == 15)) {
    if (may_page_fault) {
      klongjmp(&page_fault_handler);
    }
  }

  kernel_initialized = false;
  dump_csr_s();
  panic("unable to handle kernel interrupt");
}

__noreturn void trap_handler () {
  if (read_sstatus().spp) {
    klongjmp(kernel_trap(&trapframe.task));
  }

  if (handle_interrupt(&trapframe.task, MODE_USER) == 0) {
    return_to_user();
  }

  if (!read_scause().interrupt) {
    current_task->user_frame = trapframe.task;
    klongjmp(&current_task->kernel_frame);
  }

  kernel_initialized = false;
  dump_csr_s();
  panic("unable to handle user interrupt");
}

void irq_init () {
  printk("Initializing interrupts.\n");
  csrw("stvec", trap_vector);
  struct interrupt_bitmap sie = { .value = 0 };
  sie.ssi = true;
  sie.sei = true;
  csrw("sie", sie);

  may_page_fault = false;

  __asm__ volatile ("mv %0, sp" : "=r" (trapframe.kernel_sp));

  *(u32 *)(PLIC + UART0_IRQ * 4) = 1;
  *(u32 *)PLIC_SENABLE(0) = (1 << UART0_IRQ);
  *(u32 *)PLIC_SPRIORITY(0) = 0;
}


__noreturn void return_to_user () {
  // turn off interrupts
  struct status sstatus = read_sstatus();
  sstatus.spp = 0;
  sstatus.sie = false;
  sstatus.spie = true;
  write_sstatus(sstatus);

  _kernel_to_user();
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
