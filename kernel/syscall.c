#include "syscall.h"

#include "elf.h"
#include "errno.h"
#include "printf.h"
#include "riscv.h"
#include "sched.h"
#include "trap.h"
#include "uart.h"
#include "vm.h"

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

  case SYS_yield:
    current_task->mode = MODE_SUPERVISOR;
    if (ksetjmp(&current_task->kernel_frame)) {
      return 0;
    }
    schedule_next();

  case SYS_fork: {
    let task = task_clone(current_task);
    if (!task) {
      return -EAGAIN;
    }
    task->user_frame.registers[REG_A0] = 0;
    return task->pid;
  }

  case SYS_exit:
    task_destroy(current_task);
    schedule();

  case SYS_exec: {
    // TODO
    elf program = (elf) VA((void __phy *) 0x800f0000L);
    struct task *task = task_create(current_task);
    load_elf(task, program);
    task->user_frame.pc = program->e_entry;
    return 0;
  }

  case 114514:
    vm_dump(&current_task->vm_areas);
    return 0;

  default:
    return -ENOSYS;
  }
}
