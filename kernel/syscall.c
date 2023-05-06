#include "syscall.h"

#include "errno.h"
#include "printf.h"
#include "riscv.h"
#include "uart.h"

u64 syscall (struct task *task) {
  u64 id = task->user_frame.registers[REG_A7];
  u64 a0 = task->user_frame.registers[REG_A0];
  // u64 a1 = task->user_frame.registers[REG_A1];
  // u64 a2 = task->user_frame.registers[REG_A2];
  // u64 a3 = task->user_frame.registers[REG_A3];
  // u64 a4 = task->user_frame.registers[REG_A4];
  // u64 a5 = task->user_frame.registers[REG_A5];
  switch (id) {
  case SYS_getchar:
    return getchar();

  case SYS_putchar:
    putchar(a0);
    return 0;

  case SYS_getpid:
    return task->pid;

  default:
    return -ENOSYS;
  }
}
