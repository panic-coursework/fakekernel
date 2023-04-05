#include "memlayout.h"

	.section	.text.kernelvec
	.align	2
	.globl	kernelvec
kernelvec:
	addi	sp, sp, -256

	sd      x1, 8(sp)
	sd      x2, 16(sp)
	sd      x3, 24(sp)
	sd      x4, 32(sp)
	sd      x5, 40(sp)
	sd      x6, 48(sp)
	sd      x7, 56(sp)
	sd      x8, 64(sp)
	sd      x9, 72(sp)
	sd      x10, 80(sp)
	sd      x11, 88(sp)
	sd      x12, 96(sp)
	sd      x13, 104(sp)
	sd      x14, 112(sp)
	sd      x15, 120(sp)
	sd      x16, 128(sp)
	sd      x17, 136(sp)
	sd      x18, 144(sp)
	sd      x19, 152(sp)
	sd      x20, 160(sp)
	sd      x21, 168(sp)
	sd      x22, 176(sp)
	sd      x23, 184(sp)
	sd      x24, 192(sp)
	sd      x25, 200(sp)
	sd      x26, 208(sp)
	sd      x27, 216(sp)
	sd      x28, 224(sp)
	sd      x29, 232(sp)
	sd      x30, 240(sp)
	sd      x31, 248(sp)

	call	kernel_trap

	ld      x1, 8(sp)
	ld      x2, 16(sp)
	ld      x3, 24(sp)
	ld      x4, 32(sp)
	ld      x5, 40(sp)
	ld      x6, 48(sp)
	ld      x7, 56(sp)
	ld      x8, 64(sp)
	ld      x9, 72(sp)
	ld      x10, 80(sp)
	ld      x11, 88(sp)
	ld      x12, 96(sp)
	ld      x13, 104(sp)
	ld      x14, 112(sp)
	ld      x15, 120(sp)
	ld      x16, 128(sp)
	ld      x17, 136(sp)
	ld      x18, 144(sp)
	ld      x19, 152(sp)
	ld      x20, 160(sp)
	ld      x21, 168(sp)
	ld      x22, 176(sp)
	ld      x23, 184(sp)
	ld      x24, 192(sp)
	ld      x25, 200(sp)
	ld      x26, 208(sp)
	ld      x27, 216(sp)
	ld      x28, 224(sp)
	ld      x29, 232(sp)
	ld      x30, 240(sp)
	ld      x31, 248(sp)

	addi	sp, sp, 256
	sret


	.section	._early.trapframe
	.align	12
	.globl	trapframe
trapframe:
	.zero	PAGE_SIZE

	.section	._trampoline
	.align	12
	.globl	trampoline
trampoline:

	.globl	_user_to_kernel
_user_to_kernel:
	csrw	sscratch, t0
	li	t0, TRAPFRAME

	sd      x1, 8(t0)
	sd      x2, 16(t0)
	sd      x3, 24(t0)
	sd      x4, 32(t0)
	sd      x6, 48(t0)
	sd      x7, 56(t0)
	sd      x8, 64(t0)
	sd      x9, 72(t0)
	sd      x10, 80(t0)
	sd      x11, 88(t0)
	sd      x12, 96(t0)
	sd      x13, 104(t0)
	sd      x14, 112(t0)
	sd      x15, 120(t0)
	sd      x16, 128(t0)
	sd      x17, 136(t0)
	sd      x18, 144(t0)
	sd      x19, 152(t0)
	sd      x20, 160(t0)
	sd      x21, 168(t0)
	sd      x22, 176(t0)
	sd      x23, 184(t0)
	sd      x24, 192(t0)
	sd      x25, 200(t0)
	sd      x26, 208(t0)
	sd      x27, 216(t0)
	sd      x28, 224(t0)
	sd      x29, 232(t0)
	sd      x30, 240(t0)
	sd      x31, 248(t0)
	csrr	t1, sscratch
	sd	t1, 40(t0)
	csrr	t1, sepc
	sd	t1, 280(t0)

	ld	t1, 256(t0)
	sfence.vma	x0, x0
	csrw	satp, t1
	sfence.vma	x0, x0
	ld	sp, 264(t0)

	ld	t0, 272(t0)
	jr	t0


	.align	2
	.globl	_kernel_to_user
_kernel_to_user:
	li	t0, TRAPFRAME
	ld	t1, 280(t0)
	csrw	sepc, t1
	sd	sp, 264(t0)

	sfence.vma	x0, x0
	csrw	satp, a0
	sfence.vma	x0, x0

	ld	t1, 40(t0)
	csrw	sscratch, t1

	ld      x1, 8(t0)
	ld      x2, 16(t0)
	ld      x3, 24(t0)
	ld      x4, 32(t0)
	ld      x6, 48(t0)
	ld      x7, 56(t0)
	ld      x8, 64(t0)
	ld      x9, 72(t0)
	ld      x10, 80(t0)
	ld      x11, 88(t0)
	ld      x12, 96(t0)
	ld      x13, 104(t0)
	ld      x14, 112(t0)
	ld      x15, 120(t0)
	ld      x16, 128(t0)
	ld      x17, 136(t0)
	ld      x18, 144(t0)
	ld      x19, 152(t0)
	ld      x20, 160(t0)
	ld      x21, 168(t0)
	ld      x22, 176(t0)
	ld      x23, 184(t0)
	ld      x24, 192(t0)
	ld      x25, 200(t0)
	ld      x26, 208(t0)
	ld      x27, 216(t0)
	ld      x28, 224(t0)
	ld      x29, 232(t0)
	ld      x30, 240(t0)
	ld      x31, 248(t0)
	csrr	t0, sscratch

	sret
