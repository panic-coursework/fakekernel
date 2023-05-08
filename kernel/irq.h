#pragma once

#include "mm.h"
#include "trap.h"

void irq_init ();

__attribute__((noreturn)) void return_to_user ();

void dump_cpu (struct cpu *cpu);
