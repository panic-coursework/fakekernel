#pragma once

#include <stdbool.h>

#include "type.h"

extern bool panicked;
__noreturn void _panic (const char *fmt, ...);
void kassert (bool cond, const char *message);

#define panic(...) \
  _panic("%s:%d: %s: %s", __FILE__, __LINE__, __func__, __VA_ARGS__)
