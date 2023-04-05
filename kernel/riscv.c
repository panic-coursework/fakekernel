#include "riscv.h"

#include "printf.h"

#define dump(x) do {       \
  u64 v;                   \
  csrr(x, v);              \
  printk("%p " x "\n", v); \
} while (false)

void dump_csr_s () {
  dump("sstatus");
  dump("sie");
  dump("stvec");
  dump("scounteren");
  dump("senvcfg");
  dump("sscratch");
  dump("sepc");
  dump("scause");
  dump("stval");
  dump("sip");
  dump("satp");
}

void dump_csr_m () {
  dump("mvendorid");
  dump("marchid");
  dump("mimpid");
  dump("mhartid");
  dump("mconfigptr");
  dump("mstatus");
  dump("misa");
  dump("medeleg");
  dump("mideleg");
  dump("mie");
  dump("mtvec");
  dump("mcounteren");
  dump("mscratch");
  dump("mepc");
  dump("mcause");
  dump("mtval");
  dump("mip");
  dump("mtinst");
}

void dump_csr_all () {
  dump_csr_m();
  dump_csr_s();
}
