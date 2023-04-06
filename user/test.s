	.text
	.globl	_start
_start:
	li	a7, 2
	ecall
	addi	s0, a0, '0'
.Lloop:
	mv	a0, s0
	li	a7, 1
	ecall
	j	.Lloop
