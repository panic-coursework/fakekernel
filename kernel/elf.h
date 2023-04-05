#pragma once

#include </usr/include/elf.h>

#include "mm.h"

typedef Elf64_Ehdr *elf;

page_table_t load_elf (elf program);
