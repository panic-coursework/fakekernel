#pragma once

#include "memlayout.h"
#include "mm.h"
#include "type.h"

struct cpu {
  u64 registers[32];
  u64 pc;
};

int ksetjmp (struct cpu *cpu);
__noreturn void klongjmp (struct cpu *cpu);

struct trapframe {
  struct cpu task;
  u64 kernel_sp;
};
_Static_assert(sizeof(struct trapframe) < 512, "trapframe too large");
extern struct trapframe trapframe;

__noreturn void trap_vector ();
__noreturn void _kernel_to_user ();
