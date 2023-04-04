#include "printf.h"

#include <stdarg.h>
#include <stdbool.h>

void write (const char *buf, size_t length) {
  for (size_t i = 0; i < length; ++i) {
    putchar(buf[i]);
  }
}

static void printInt (i64 x) {
  printk("unimplemented");
}
static void printHex (i64 x) {
  printk("unimplemented");
}
static void printPointer (void *x) {
  printk("unimplemented");
}

void printk (const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  char c;
  while ((c = *fmt++)) {
    if (c != '%') {
      putchar(c);
      continue;
    }
    
    c = *fmt++;
    switch (c) {
    case '%':
      putchar('%');
      break;

    case 'd':
      printInt(va_arg(args, int));
      break;

    case 'x':
      printHex(va_arg(args, int));
      break;

    case 'p':
      printPointer(va_arg(args, void *));
      break;

    case 's': {
      const u8 *str = va_arg(args, const u8 *);
      u8 c1;
      while ((c1 = *str++)) {
        putchar(c1);
      }
      break;
    }

    case '\0':
      break;

    default:
      putchar(c);
      break;
    }
  }

  va_end(args);
}
