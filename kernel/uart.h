#pragma once

void uart_init ();
void uartputc (int c);
void uartstart ();
int uartgetc ();
int getchar ();
void uartintr ();
