#include "stdio.h"

int main () {
  putchar('H');
  putchar('e');
  putchar('l');
  putchar('l');
  putchar('o');
  return 0;
}

void _start () {
  main();
  while (true) continue;
}
