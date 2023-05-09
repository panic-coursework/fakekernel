#include "stdio.h"
#include "syscall.h"

#define MAXN 10010
int dp[MAXN];

int fact (int x) {
  if (x == 0) return 0;
  int t = fact(x - 1) + x;
  dp[x] = t;
  return t;
}

void _start () {
  putchar('f'); putchar('o'); putchar('r'); putchar('k'); putchar(':'); putchar(' ');
  putint(internal_syscall0(4));
  putchar('\n');
  putchar('p'); putchar('i'); putchar('d'); putchar(':'); putchar(' ');
  putint(internal_syscall0(2));
  putchar('\n');
  internal_syscall0(114514);
  if (internal_syscall0(2) == 1) {
    internal_syscall0(6);
    while (1) internal_syscall0(3);
  } else {
    internal_syscall0(5);
  }
  int x = *(int *)0x114514;
  while (true) {
    putint(internal_syscall0(2));
    // internal_syscall0(3);
  }
  putint(fact(getint()));
  putchar('\n');
  internal_syscall0(114514);
  while (true);
}
