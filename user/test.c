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
  putint(fact(getint()));
  putchar('\n');
  internal_syscall0(114514);
  while (true);
}
