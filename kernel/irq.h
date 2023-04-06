#pragma once

#include "mm.h"
#include "trap.h"

void irq_init ();

__attribute__((noreturn)) void return_to_user (page_table_t table);

void dump_cpu (struct cpu *cpu);
