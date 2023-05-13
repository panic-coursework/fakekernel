#pragma once

#include </usr/include/elf.h>

#include "mm.h"
#include "sched.h"

typedef const Elf64_Ehdr *elf;

int load_elf (struct task *task, elf program);
int task_init_elf (struct task *task, elf program, u8 **argv, u8 **envp);
