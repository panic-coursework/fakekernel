#include <stdbool.h>

#include "mm.h"
#include "printf.h"
#include "uart.h"

char stack[0x1000];
bool panicked = false;

extern void mret();
extern void idle();

void timer () {
  printk("timer\n");
}
void timerret () {
  printk("timer return\n");
}

void main () {
  uartinit();

  printk("Hello %s %s %s!\n", "Brave", "New", "World");

  establish_identical_mapping();
  printk("Identical mapping established, entering S-mode\n");
  mret();
  printk("Entered S-mode.\n");

  idle();
}
