#include "memlayout.h"

	.section	._start
	.globl	_start
_start:
	la	sp, stack
	li	t0, 0x1000
	add	sp, sp, t0
	call	main


	.align	8
	.text
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
	lw	t0, 0(t0)
	li	t1, TIMER_INTERVAL
	add	t1, t1, t0
	li	t0, CLINT_MTIMECMP(0)
	sw	t1, 0(t0)

	la	t0, timervec
	csrw	mtvec, t0
	csrw	stvec, t0

	li	t0, 0xa0
	csrw	mie, t0

	# return to S-mode, and enable interrupts (for timer)
	csrr	t0, mstatus
	li	t1, 0x80a
	or	t0, t0, t1
	csrw	mstatus, t0

	# return
	csrw	mepc, ra
	mret


	.align	8
	.globl 	timervec
timervec:
	li	t0, CLINT_MTIME
	lw	t0, 0(t0)
	li	t1, TIMER_INTERVAL
	add	t1, t1, t0
	li	t0, CLINT_MTIMECMP(0)
	sw	t1, 0(t0)

	call	timer
	li	t0, 1
	mret


	.align	8
	.globl	idle
idle:
	li	t0, 0
	wfi
	beqz	t0, idle
	call	timerret
	j	idle
