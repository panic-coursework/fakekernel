	.text
	.globl	_start
_start:
	li	t0, 0
	li	t1, 100
	li	t2, 0
.Lloop:
	bgt	t0, t1, .Lend
	add	t2, t2, t0
	addi	t0, t0, 1
	j	.Lloop
.Lend:
	mv	a0, t2
	ecall
