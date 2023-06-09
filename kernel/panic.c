#include "panic.h"

#include <stdarg.h>

#include "main.h"
#include "printf.h"

bool panicked = false;

__noreturn void _panic (const char *fmt, ...) {
  kernel_initialized = false;

  va_list args;
  va_start(args, fmt);

  printk("kernel panic - ");
  vprintk(fmt, args);
  printk("\n");

  panicked = true;
  while (true) continue;
}

void kassert (bool cond, const char *message) {
  if (!cond) {
    _panic("assertion failed: %s", message);
  }
}
