#pragma once

#include "sched.h"
#include "type.h"

u64 syscall (struct task *task);

#define SYS_getchar 0
#define SYS_putchar 1
#define SYS_getpid  2
#define SYS_yield   3
#define SYS_fork    4
#define SYS_exit    5
#define SYS_exec    6
