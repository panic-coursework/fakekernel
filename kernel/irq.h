#pragma once

#include "mm.h"

void irq_init ();

__attribute__((noreturn)) void return_to_user (page_table_t table);
