#include "panic.h"

#include "printf.h"

bool panicked = false;

__attribute__((noreturn)) void panic (const char *message) {
  printk("kernel panic - %s\n", message);
  panicked = true;
  while (true) continue;
}
