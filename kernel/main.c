#include <stdbool.h>

#include "elf.h"
#include "irq.h"
#include "memlayout.h"
#include "mm.h"
#include "printf.h"
#include "start.h"
#include "sched.h"
#include "trap.h"
#include "uart.h"

bool kernel_initialized;

__attribute__((noreturn)) void main () { // NOLINT
  uart_init();

  printk("Hello %s %s %s!\n", "Brand", "New", "World");

  establish_identical_mapping();
  printk("Identity mapping established, entering S-mode\n");
  mret();

  mm_init();
  irq_init();
  sched_init();

  elf program = (elf) 0x800f0000L;
  struct task *task = create_task(NULL, load_elf(program));
  task->user_frame.pc = program->e_entry;

  task = create_task(NULL, load_elf(program));
  task->user_frame.pc = program->e_entry;

  kernel_initialized = true;

  schedule();
}
