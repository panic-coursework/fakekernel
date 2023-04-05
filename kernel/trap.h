#pragma once

#include "memlayout.h"
#include "mm.h"
#include "type.h"

void kernelvec ();

struct trapframe {
  u64 registers[32];
  u64 kernel_satp;
  u64 kernel_sp;
  u64 user_trap;
  u64 sepc;
};
_Static_assert(sizeof(struct trapframe) < PAGE_SIZE, "trapframe too large");

extern struct trampoline trampoline;
extern struct trapframe trapframe;

__attribute__((noreturn)) void _user_to_kernel ();
__attribute__((noreturn)) void _kernel_to_user (u64 satp);
