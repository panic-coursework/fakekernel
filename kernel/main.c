#include <stdbool.h>

#include "elf.h"
#include "irq.h"
#include "memlayout.h"
#include "mm.h"
#include "printf.h"
#include "start.h"
#include "trap.h"
#include "uart.h"

void main () {
  uart_init();

  printk("Hello %s %s %s!\n", "Brand", "New", "World");

  establish_identical_mapping();
  printk("Identity mapping established, entering S-mode\n");
  mret();

  mm_init();
  irq_init();

  elf program = (elf) 0x800f0000L;
  trapframe.sepc = program->e_entry;
  return_to_user(load_elf(program));

  printk("Idle.\n");
  idle();
}
