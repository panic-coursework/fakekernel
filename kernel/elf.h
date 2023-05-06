#pragma once

#include </usr/include/elf.h>

#include "mm.h"
#include "sched.h"

typedef Elf64_Ehdr *elf;

int load_elf (struct task *task, elf program);
