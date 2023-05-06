	.text
	.globl	_start
_start:
	li	a7, 2
	ecall
	mv	s0, a0
.Lloop:
	mv	a0, s0
	li	a7, 0
	ecall
	add	a0, a0, s0
	li	a7, 1
	ecall
	j	.Lloop
