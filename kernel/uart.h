#pragma once

#include "type.h"

void uart_init ();
void uartputc (int c);
void uartstart ();
int uartgetc ();
u8 getchar ();
void uartintr ();
