.code32
.text
.globl	zfunction

zfunction:

	pushl	%ebp

	movl	%esp, %ebp

	andl	$-261633, 8(%ebp)
	sarl	$8, 8(%ebp)
	movl	8(%ebp), %eax
	orl	$-16777216, %eax
	movl	%eax, 8(%ebp)
	movl	8(%ebp), %eax
	popl	%ebp

	ret
