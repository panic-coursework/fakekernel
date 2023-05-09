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

void _start (int argc, char **argv, char **envp) {
  putstr("argc: ");
  putint(argc);
  putchar('\n');
  for (int i = 0; i < argc; ++i) {
    putstr("argv");
    putint(i);
    putstr(": ");
    putstr(argv[i]);
    putchar('\n');
  }
  int fork_pid = internal_syscall0(4);
  putstr("pid: ");
  putint(internal_syscall0(2));
  putchar('\n');
  putstr("fork: ");
  putint(fork_pid);
  putchar('\n');
  if (internal_syscall0(2) == 1) {
    if (argv[0][0] == 'i') {
      const char *argv[] = {"/test", "22", "33", 0};
      const char *envp[] = {0};
      putstr("execve: ");
      putint(internal_syscall3(6, 0, argv, envp));
      putchar('\n');
    }
    putchar('\n');
    while (1) internal_syscall0(3);
  } else {
    putchar('\n');
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
