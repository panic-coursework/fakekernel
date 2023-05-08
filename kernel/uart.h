#pragma once

#include "type.h"

extern u64 uart_base;

void uart_init ();
void uartputc (int c);
void uartstart ();
int uartgetc ();
u8 getchar ();
void uartintr ();
