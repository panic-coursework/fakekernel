// This file is part of xv6[1], modified by Alan Liang.
// Reproduced under the MIT license.
// [1]: https://github.com/mit-pdos/xv6-riscv/blob/f5b93ef12f7159f74f80f94729ee4faabe42c360/kernel/memlayout.h

#pragma once

// Physical memory layout

// qemu -machine virt is set up like this,
// based on qemu's hw/riscv/virt.c:
//
// 00001000 -- boot ROM, provided by qemu
// 02000000 -- CLINT
// 0C000000 -- PLIC
// 10000000 -- uart0 
// 10001000 -- virtio disk 
// 80000000 -- boot ROM jumps here in machine mode
//             -kernel loads the kernel here
// unused RAM after 80000000.

// the kernel uses physical memory thus:
// 80000000 -- entry.S, then kernel text and data
// end -- start of kernel page allocation area
// PHYSTOP -- end RAM used by the kernel

// qemu puts UART registers here in physical memory.
#define UART0 0x10000000L
#define UART0_IRQ 10

// virtio mmio interface
#define VIRTIO0 0x10001000
#define VIRTIO0_IRQ 1

// core local interruptor (CLINT), which contains the timer.
#define CLINT 0x2000000L
#define CLINT_MTIMECMP(hartid) (CLINT + 0x4000 + 8*(hartid))
#define CLINT_MTIME (CLINT + 0xBFF8) // cycles since boot.

// qemu puts platform-level interrupt controller (PLIC) here.
// virtual address here; not used in M-mode
#define PLIC (MMIOBASE + 0x0c000000L)
#define PLIC_PRIORITY (PLIC + 0x0)
#define PLIC_PENDING (PLIC + 0x1000)
#define PLIC_MENABLE(hart) (PLIC + 0x2000 + (hart)*0x100)
#define PLIC_SENABLE(hart) (PLIC + 0x2080 + (hart)*0x100)
#define PLIC_MPRIORITY(hart) (PLIC + 0x200000 + (hart)*0x2000)
#define PLIC_SPRIORITY(hart) (PLIC + 0x201000 + (hart)*0x2000)
#define PLIC_MCLAIM(hart) (PLIC + 0x200004 + (hart)*0x2000)
#define PLIC_SCLAIM(hart) (PLIC + 0x201004 + (hart)*0x2000)

// the kernel expects there to be RAM
// for use by the kernel and user pages
// from physical address 0x80000000 to PHYSTOP.
#define PHYBASE 0x80000000L
#define PHYSIZE (128*1024*1024)
#define PHYSTOP (PHYBASE + PHYSIZE)
#define PAGE_INDEX_BITS 12
#define PAGE_SIZE (1 << PAGE_INDEX_BITS)
#define MAX_ORDER 10
#define SKIP_ORDER 8 // 256 * 4K = 1M

#define MAXVA (1L << (9 + 9 + 9 + 12 - 1))
#define SPLIT    0x3f00000000L
#define KERNPHY  (PHYBASE)
#define KERNBASE (SPLIT)
// #define KERNBASE (PHYBASE)
#define KERNSTOP (KERNBASE + 0x40000000L)
#define KERN_PHYBASE (KERNBASE)
#define KERN_PHYSTOP (KERN_PHYBASE + PHYSIZE)
#define MMIOPHY  0x0
#define MMIOBASE (KERNSTOP)
#define MMIOSTOP (MMIOBASE + 0x40000000L)
#define USERSTACK    0x3ef0000000L
#define USERSTACKMIN 0x3ee0000000L

#define _PA(x) ((x) - KERNBASE + KERNPHY)
#define _VA(x) ((x) + KERNBASE - KERNPHY)
