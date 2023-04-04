#pragma once

#include <stdbool.h>

extern bool panicked;
__attribute__((noreturn)) void panic (const char *message);
