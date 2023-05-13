#include "syscall.h"

#include "elf.h"
#include "errno.h"
#include "irq.h"
#include "mm.h"
#include "mmdefs.h"
#include "printf.h"
#include "riscv.h"
#include "sched.h"
#include "trap.h"
#include "type.h"
#include "uart.h"
#include "vm.h"

u64 syscall (struct task *task) {
  u64 id = task->user_frame.registers[REG_A7];
  u64 a0 = task->user_frame.registers[REG_A0];
  u64 a1 = task->user_frame.registers[REG_A1];
  u64 a2 = task->user_frame.registers[REG_A2];
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
    if (IS_ERR(task)) {
      return PTR_ERR(task);
    }
    task->user_frame.registers[REG_A0] = 0;
    return task->pid;
  }

  case SYS_exit:
    task_destroy(current_task);
    schedule();

  case SYS_execve: {
    u64 retval;

    // TODO
    elf program = (elf) VA((void __phy *) 0x800f0000L);

    u8 **argv = user_copy_args((u8 * __user *) a1);
    if (IS_ERR(argv)) {
      retval = PTR_ERR(argv);
      goto err_execve;
    }
    u8 **envp = user_copy_args((u8 * __user *) a2);
    if (IS_ERR(argv)) {
      retval = PTR_ERR(argv);
      goto err_execve_argv;
    }

    retval = task_reinit(current_task);
    if (retval) goto err_execve_task;

    retval = task_init_elf(task, program, argv, envp);
    if (retval) goto err_execve_task;

    // not "return 0" here, as exec creates a new kernel thread.
    klongjmp(&current_task->kernel_frame);

    err_execve_task:
    kfree(argv);
    kfree(envp);
    task_destroy(current_task);
    panic("unreachable");

    err_execve_argv: kfree(argv);
    err_execve: return retval;
  }

  case 114514:
    vm_dump(&current_task->vm_areas);
    return 0;

  default:
    return -ENOSYS;
  }
}
