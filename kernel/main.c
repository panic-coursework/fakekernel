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

bool early = true;
bool kernel_initialized;

__noreturn static void init ();
__noreturn void main () { // NOLINT
  uart_init();

  printk("Hello %s %s %s!\n", "Brand", "New", "World");

  establish_identical_mapping();
  printk("Identity mapping established, entering S-mode\n");
  mret();
  init();

} // barrier for optimizations
__attribute__((noreturn, noinline)) static void init () {

  early = false;
  uart_base += MMIOBASE;

  mm_init();
  irq_init();
  sched_init();

  elf program = (elf) VA((void __phy *) 0x800f0000L);
  struct task *task = task_create(NULL);
  load_elf(task, program);
  task->user_frame.pc = program->e_entry;

  kernel_initialized = true;

  schedule();
}
