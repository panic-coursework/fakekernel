#pragma once

#include <stdbool.h>

#include "syscall.h"

static inline void putchar (char c) {
  internal_syscall1(1, c);
}

static inline char getchar (void) {
  return internal_syscall0(0);
}

static bool isdigit (char c) {
  return c >= '0' && c <= '9';
}

#define BUFLEN     64

static void print_uint (unsigned long long x, bool neg) {
  char buf[BUFLEN] = {0};
  int top = 0;
  do {
    buf[top++] = (x % 10) + '0';
    x /= 10;
  } while (x != 0);
  if (neg) putchar('-');
  while (top >= 0) putchar(buf[top--]);
}

static inline void putint (int value) {
  if (value < 0) {
    print_uint(-(long long) value, true);
  } else {
    print_uint(value, false);
  }
}

static inline int getint () {
  int i = 0, sign = 1;
  char c;
  while (!isdigit(c = getchar())) {
    if (c == '-') sign = -1;
  }
  do {
    i = i * 10 + c - '0';
  } while (isdigit(c = getchar()));
  return i * sign;
}

static inline void putstr (const char *s) {
  while (*s) putchar(*s++);
}

static inline void yield () {
  internal_syscall0(3);
}
