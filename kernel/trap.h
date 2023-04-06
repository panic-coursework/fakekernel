#pragma once

#include "memlayout.h"
#include "mm.h"
#include "type.h"

struct cpu {
  u64 registers[32];
  u64 pc;
};

__attribute__((noreturn)) void kernelvec ();
int ksetjmp (struct cpu *cpu);
__attribute__((noreturn)) void klongjmp (struct cpu *cpu);

struct trapframe {
  struct cpu task;
  u64 kernel_sp;
  u64 kernel_satp;
  u64 user_trap;
};
_Static_assert(sizeof(struct trapframe) < PAGE_SIZE, "trapframe too large");

extern struct trampoline trampoline;
extern struct trapframe trapframe;

__attribute__((noreturn)) void _user_to_kernel ();
__attribute__((noreturn)) void _kernel_to_user (u64 satp);
