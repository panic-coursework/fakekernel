#include "printf.h"

#include <stdbool.h>
#include <stdint.h>

void write (const char *buf, size_t length) {
  for (size_t i = 0; i < length; ++i) {
    putchar(buf[i]);
  }
}

static void prints (const char *str) {
  u8 c;
  while ((c = *str++)) {
    putchar(c);
  }
}

static bool isdigit (char c) {
  return c >= '0' && c <= '9';
}

static int scan_int (const char **str) {
  int i = 0, sign = 1;
  char c;
  while (!isdigit(c = *(*str)++)) {
    if (c == '-') sign = -1;
  }
  do {
    i = i * 10 + c - '0';
  } while (isdigit(c = *(*str)++));
  --*str;
  return i * sign;
}

#define P_UNSIGNED (1 << 0)
#define P_HEX      (1 << 1)
#define P_BIN      (1 << 2)
#define P_ZEROPAD  (1 << 3)
#define BUFLEN     64

static void print_uint (u64 x, int base, int pad, char padchar, bool neg) {
  char buf[BUFLEN] = {0};
  int top = 0;
  const char *tab = "0123456789abcdef";
  do {
    buf[top++] = tab[x % base];
    x /= base;
  } while (x != 0);

  if (neg && padchar == '0') putchar('-');
  for (int i = top + neg; i < pad; ++i) putchar(padchar);
  if (neg && padchar == ' ') putchar('-');

  while (top >= 0) putchar(buf[top--]);
}

static void print_int (u64 x, int flags, int pad) {
  char padchar = (flags & P_ZEROPAD) ? '0' : ' ';
  
  int base = (flags & P_HEX) ? 16 : (flags & P_BIN) ? 2 : 10;
  if (flags & P_UNSIGNED) {
    print_uint(x, base, pad, padchar, false);
    return;
  }
  if (x == (u64) INT64_MIN) {
    if (padchar == '0') putchar('-');
    for (int i = 20; i < pad; ++i) putchar(padchar);
    if (padchar == ' ') putchar('-');
    prints("9223372036854775808");
    return;
  }

  i64 y = x;
  print_uint(y < 0 ? -y : y, base, pad, padchar, y < 0);
}

static u64 next_arg (va_list *args, bool is64) {
  if (is64) {
    return va_arg(*args, u64);
  } else {
    return va_arg(*args, u32);
  }
}

void vprintk (const char *fmt, va_list args) {
  char c;
  while ((c = *fmt++)) {
    if (c != '%') {
      putchar(c);
      continue;
    }
    
    int flags = 0;
    int pad = 0;
    bool is64 = false;
    bool done = false;
    while (!done) {
      done = true;
      c = *fmt++;
      if (c == '0') {
        flags |= P_ZEROPAD;
        done = false;
        continue;
      } else if (isdigit(c)) {
        --fmt;
        pad = scan_int(&fmt);
        done = false;
        continue;
      }
      switch (c) {
      case '%':
        putchar('%');
        break;

      case 'd':
        print_int(next_arg(&args, is64), flags, pad);
        break;

      case 'x':
        print_int(next_arg(&args, is64), flags | P_HEX | P_UNSIGNED, pad);
        break;

      case 'a':
        print_int(next_arg(&args, is64), flags | P_HEX, pad);
        break;

      case 'b':
        print_int(next_arg(&args, is64), flags | P_BIN, pad);
        break;

      case 'u':
        print_int(next_arg(&args, is64), flags | P_UNSIGNED, pad);
        done = false;
        break;

      case 'l':
        is64 = true;
        done = false;
        break;

      case 'p':
        pad = 16;
        print_int(va_arg(args, u64), flags | P_UNSIGNED | P_HEX | P_ZEROPAD, pad);
        break;

      case 'c':
        putchar(va_arg(args, int));
        break;

      case 's':
        prints(va_arg(args, const char *));
        break;

      case '\0':
        break;

      default:
        putchar(c);
        break;
      }
    }
  }

  va_end(args);
}

void printk (const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vprintk(fmt, args);
}
