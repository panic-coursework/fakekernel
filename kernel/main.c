#include <stdbool.h>

#include "irq.h"
#include "memlayout.h"
#include "mm.h"
#include "printf.h"
#include "start.h"
#include "trampoline.h"
#include "uart.h"

void timer () {
#ifdef TIMER_DEBUG
  u64 time = *(u64 *)CLINT_MTIME;
  printk("timer at %10lu\n", time);
#endif
}
void timerret () {
#ifdef TIMER_DEBUG
  printk("timer return\n");
#endif
}

void main () {
  uart_init();

  printk("Hello %s %s %s!\n", "Brand", "New", "World");

  establish_identical_mapping();
  printk("Identity mapping established, entering S-mode\n");
  mret();

  mm_init();
  irq_init();

  printk("Idle.\n");
  idle();
}
