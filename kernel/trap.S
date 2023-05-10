#include "memlayout.h"

	.section	.text.ksetjmp
	.globl	ksetjmp
ksetjmp:
	sd	x1, 8(a0)
	sd	x2, 16(a0)
	sd	x3, 24(a0)
	sd	x4, 32(a0)
	sd	x5, 40(a0)
	sd	x6, 48(a0)
	sd	x7, 56(a0)
	sd	x8, 64(a0)
	sd	x9, 72(a0)
	sd	x10, 80(a0)
	sd	x11, 88(a0)
	sd	x12, 96(a0)
	sd	x13, 104(a0)
	sd	x14, 112(a0)
	sd	x15, 120(a0)
	sd	x16, 128(a0)
	sd	x17, 136(a0)
	sd	x18, 144(a0)
	sd	x19, 152(a0)
	sd	x20, 160(a0)
	sd	x21, 168(a0)
	sd	x22, 176(a0)
	sd	x23, 184(a0)
	sd	x24, 192(a0)
	sd	x25, 200(a0)
	sd	x26, 208(a0)
	sd	x27, 216(a0)
	sd	x28, 224(a0)
	sd	x29, 232(a0)
	sd	x30, 240(a0)
	sd	x31, 248(a0)

	la	t0, .Lret1
	sd	t0, 256(a0)
	li	a0, 0
	ret

.Lret1:
	ld	ra, 8(a0)
	li	a0, 1
	ret

	.section	.text.klongjmp
	.globl	klongjmp
klongjmp:
	ld	ra, 256(a0)
	ld	x2, 16(a0)
	ld	x3, 24(a0)
	ld	x4, 32(a0)
	ld	x5, 40(a0)
	ld	x6, 48(a0)
	ld	x7, 56(a0)
	ld	x8, 64(a0)
	ld	x9, 72(a0)
	ld	x11, 88(a0)
	ld	x12, 96(a0)
	ld	x13, 104(a0)
	ld	x14, 112(a0)
	ld	x15, 120(a0)
	ld	x16, 128(a0)
	ld	x17, 136(a0)
	ld	x18, 144(a0)
	ld	x19, 152(a0)
	ld	x20, 160(a0)
	ld	x21, 168(a0)
	ld	x22, 176(a0)
	ld	x23, 184(a0)
	ld	x24, 192(a0)
	ld	x25, 200(a0)
	ld	x26, 208(a0)
	ld	x27, 216(a0)
	ld	x28, 224(a0)
	ld	x29, 232(a0)
	ld	x30, 240(a0)
	ld	x31, 248(a0)
	ld	a0, 80(a0)

	ret


	.section	._early.trapframe
	.align	2
	.globl	trapframe
trapframe:
	.zero	512

	.section	.text.trap_vector
	.align	2
	.globl	trap_vector
trap_vector:
	csrw	sscratch, t0
	la	t0, trapframe

	sd	x1, 8(t0)
	sd	x2, 16(t0)
	sd	x3, 24(t0)
	sd	x4, 32(t0)
	sd	x6, 48(t0)
	sd	x7, 56(t0)
	sd	x8, 64(t0)
	sd	x9, 72(t0)
	sd	x10, 80(t0)
	sd	x11, 88(t0)
	sd	x12, 96(t0)
	sd	x13, 104(t0)
	sd	x14, 112(t0)
	sd	x15, 120(t0)
	sd	x16, 128(t0)
	sd	x17, 136(t0)
	sd	x18, 144(t0)
	sd	x19, 152(t0)
	sd	x20, 160(t0)
	sd	x21, 168(t0)
	sd	x22, 176(t0)
	sd	x23, 184(t0)
	sd	x24, 192(t0)
	sd	x25, 200(t0)
	sd	x26, 208(t0)
	sd	x27, 216(t0)
	sd	x28, 224(t0)
	sd	x29, 232(t0)
	sd	x30, 240(t0)
	sd	x31, 248(t0)
	csrr	t1, sscratch
	sd	t1, 40(t0)

	csrr	t1, sepc
	sd	t1, 256(t0)

	ld	sp, 264(t0)
	j	trap_handler


	.section	.text._kernel_to_user
	.align	2
	.globl	_kernel_to_user
_kernel_to_user:
	la	t0, trapframe
	ld	t1, 256(t0)
	csrw	sepc, t1

	ld	t1, 40(t0)
	csrw	sscratch, t1

	ld	x1, 8(t0)
	ld	x2, 16(t0)
	ld	x3, 24(t0)
	ld	x4, 32(t0)
	ld	x6, 48(t0)
	ld	x7, 56(t0)
	ld	x8, 64(t0)
	ld	x9, 72(t0)
	ld	x10, 80(t0)
	ld	x11, 88(t0)
	ld	x12, 96(t0)
	ld	x13, 104(t0)
	ld	x14, 112(t0)
	ld	x15, 120(t0)
	ld	x16, 128(t0)
	ld	x17, 136(t0)
	ld	x18, 144(t0)
	ld	x19, 152(t0)
	ld	x20, 160(t0)
	ld	x21, 168(t0)
	ld	x22, 176(t0)
	ld	x23, 184(t0)
	ld	x24, 192(t0)
	ld	x25, 200(t0)
	ld	x26, 208(t0)
	ld	x27, 216(t0)
	ld	x28, 224(t0)
	ld	x29, 232(t0)
	ld	x30, 240(t0)
	ld	x31, 248(t0)
	csrr	t0, sscratch

	sret
