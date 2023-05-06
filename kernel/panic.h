#pragma once

#include <stdbool.h>

extern bool panicked;
__attribute__((noreturn)) void panic (const char *fmt, ...);
static inline void kassert (bool cond, const char *message) {
  if (!cond) {
    panic("assertion failed: %s", message);
  }
}
