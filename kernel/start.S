#include "memlayout.h"

	.section	._early.stack
	.align	12
	.zero	PAGE_SIZE
stack:

	.section	._start
	.globl	_start
_start:
	la	sp, stack
	call	main


#define TIMER_INTERVAL 1000000

	.section	.text.mret
	.align	2
	.globl	mret
mret:
	# grant all physical memory access to S-mode
	li	t0, 0x3fffffffffffff
	csrw	pmpaddr0, t0
	li	t0, 0xf
	csrw	pmpcfg0, t0

	# delegate traps
	li	t0, -1
	csrw	medeleg, t0
	csrw	mideleg, t0

	# enable timer interrupt
	li	t0, CLINT_MTIME
	ld	t0, 0(t0)
	li	t1, TIMER_INTERVAL
	add	t1, t1, t0
	li	t0, CLINT_MTIMECMP(0)
	sd	t1, 0(t0)

	la	t0, timervec
	csrw	mtvec, t0

	li	t0, 0xa0
	csrw	mie, t0

	# return to S-mode and enable interrupts (for timer)
	csrr	t0, mstatus
	li	t1, 0x808
	or	t0, t0, t1
	csrw	mstatus, t0

	# return
	li	t0, (KERNBASE - KERNPHY)
	add	ra, ra, t0
	csrw	mepc, ra

	add	sp, sp, t0

	mret


	.bss
timerscratch:
	.zero	16

	.section	.text.timervec
	.align	2
	.globl 	timervec
timervec:
	csrw	mscratch, t0
	la	t0, timerscratch
	sd	t1, 8(t0)
	csrr	t1, mscratch
	sd	t1, 0(t0)

	li	t0, CLINT_MTIME
	ld	t0, 0(t0)
	li	t1, TIMER_INTERVAL
	add	t1, t1, t0
	li	t0, CLINT_MTIMECMP(0)
	sd	t1, 0(t0)

	li	t0, 2
	csrw	sip, t0

	la	t0, timerscratch
	ld	t1, 8(t0)
	ld	t0, 0(t0)
	mret


	.section	.text.spin
	.align	2
	.globl	spin
spin:
	j	spin
