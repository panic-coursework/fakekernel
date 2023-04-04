#pragma once

#include "type.h"
#include "uart.h"

static inline void putchar (u8 c) {
  uartputc(c);
}

void printk (const char *fmt, ...);
void write (const char *buf, size_t length);
