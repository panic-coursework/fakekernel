// This file is part of xv6[1], modified by Alan Liang.
// Reproduced under the MIT license.
//
// [1]: https://github.com/mit-pdos/xv6-riscv/blob/f5b93ef12f7159f74f80f94729ee4faabe42c360/kernel/uart.c

//
// low-level driver routines for 16550a UART.
//

#include "uart.h"

#include <stdbool.h>
#include <stdint.h>

#include "list.h"
#include "main.h"
#include "memlayout.h"
#include "panic.h"
#include "sched.h"

// the UART control registers are memory-mapped
// at address UART0. this macro returns the
// address of one of the registers.
#define Reg(reg) ((volatile unsigned char *)((early ? 0 : MMIOBASE) + UART0 + reg))

// the UART control registers.
// some have different meanings for
// read vs write.
// see http://byterunner.com/16550.html
#define RHR 0                 // receive holding register (for input bytes)
#define THR 0                 // transmit holding register (for output bytes)
#define IER 1                 // interrupt enable register
#define IER_RX_ENABLE (1<<0)
#define IER_TX_ENABLE (1<<1)
#define FCR 2                 // FIFO control register
#define FCR_FIFO_ENABLE (1<<0)
#define FCR_FIFO_CLEAR (3<<1) // clear the content of the two FIFOs
#define ISR 2                 // interrupt status register
#define LCR 3                 // line control register
#define LCR_EIGHT_BITS (3<<0)
#define LCR_BAUD_LATCH (1<<7) // special mode to set baud rate
#define LSR 5                 // line status register
#define LSR_RX_READY (1<<0)   // input is waiting to be read from RHR
#define LSR_TX_IDLE (1<<5)    // THR can accept another character to send

#define ReadReg(reg) (*(Reg(reg)))
#define WriteReg(reg, v) (*(Reg(reg)) = (v))

// the transmit output buffer.
#define UART_TX_BUF_SIZE 32
char uart_out_tx_buf[UART_TX_BUF_SIZE];
uint64_t uart_out_tx_w;
uint64_t uart_out_tx_r;
char uart_in_tx_buf[UART_TX_BUF_SIZE];
uint64_t uart_in_tx_w;
uint64_t uart_in_tx_r;

struct list wait_in, wait_out;

void uart_init (void) {
  // disable interrupts.
  WriteReg(IER, 0x00);

  // special mode to set baud rate.
  WriteReg(LCR, LCR_BAUD_LATCH);

  // LSB for baud rate of 38.4K.
  WriteReg(0, 0x03);

  // MSB for baud rate of 38.4K.
  WriteReg(1, 0x00);

  // leave set-baud mode,
  // and set word length to 8 bits, no parity.
  WriteReg(LCR, LCR_EIGHT_BITS);

  // reset and enable FIFOs.
  WriteReg(FCR, FCR_FIFO_ENABLE | FCR_FIFO_CLEAR);

  // enable transmit and receive interrupts.
  WriteReg(IER, IER_TX_ENABLE | IER_RX_ENABLE);

  list_init(&wait_in);
  list_init(&wait_out);
}

static inline bool uart_out_buffer_full () {
  return uart_out_tx_w == uart_out_tx_r + UART_TX_BUF_SIZE;
}
static inline bool uart_in_buffer_full () {
  return uart_in_tx_w == uart_in_tx_r + UART_TX_BUF_SIZE;
}

void uartputc (int c) {
  if (panicked) {
    while (true) continue;
  }

  bool idle = ReadReg(LSR) & LSR_TX_IDLE;
  if (kernel_initialized && !idle) {
    while (uart_out_buffer_full()) {
      wait(&wait_out);
    }
    uart_out_tx_buf[uart_out_tx_w % UART_TX_BUF_SIZE] = c;
    ++uart_out_tx_w;
    uartstart();
  } else {
    // wait for Transmit Holding Empty to be set in LSR.
    while((ReadReg(LSR) & LSR_TX_IDLE) == 0)
      ;
    WriteReg(THR, c);
  }
}

void uartstart (void) {
  while (1) {
    if (uart_out_tx_w == uart_out_tx_r) {
      // transmit buffer is empty.
      return;
    }

    if ((ReadReg(LSR) & LSR_TX_IDLE) == 0) {
      // the UART transmit holding register is full,
      // so we cannot give it another byte.
      // it will interrupt when it's ready for a new byte.
      return;
    }

    int c = uart_out_tx_buf[uart_out_tx_r % UART_TX_BUF_SIZE];
    uart_out_tx_r += 1;

    WriteReg(THR, c);
  }
}

int uartgetc (void) {
  if (ReadReg(LSR) & 0x01) {
    // input data is ready.
    return ReadReg(RHR);
  } else {
    return -1;
  }
}

u8 getchar () {
  while (uart_in_tx_r == uart_in_tx_w) {
    wait(&wait_in);
  }
  u8 c = uart_in_tx_buf[uart_in_tx_r % UART_TX_BUF_SIZE];
  ++uart_in_tx_r;
  return c;
}

// handle a uart interrupt, raised because input has
// arrived, or the uart is ready for more output, or
// both. called from devintr().
void uartintr (void) {
  // read and process incoming characters.
  bool has_chars = false;
  while (1) {
    int c = uartgetc();
    if (c == -1)
      break;
    if (!uart_in_buffer_full()) {
      uart_in_tx_buf[uart_in_tx_w % UART_TX_BUF_SIZE] = c;
      ++uart_in_tx_w;
    }
    has_chars = true;
  }
  if (has_chars) {
    wakeup(&wait_in);
  }

  // send buffered characters.
  uartstart();

  if (!uart_out_buffer_full()) {
    wakeup(&wait_out);
  }
}
