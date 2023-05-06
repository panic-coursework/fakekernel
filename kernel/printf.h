#pragma once

#include <stdarg.h>

#include "type.h"
#include "uart.h"

static inline void putchar (u8 c) {
  uartputc(c);
}

void vprintk (const char *fmt, va_list args);
void printk (const char *fmt, ...);
void write (const char *buf, size_t length);
