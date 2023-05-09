#pragma once

#include "mm.h"
#include "trap.h"

void irq_init ();

__noreturn void return_to_user ();

void dump_cpu (struct cpu *cpu);

extern bool may_page_fault;
extern struct cpu page_fault_handler;
