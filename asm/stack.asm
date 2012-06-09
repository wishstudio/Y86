	; stack manipulation

	.text

	irmovl	$1, %eax
	irmovl	$2, %ebx
	pushl	%ebx
	pushl	%eax
	popl	%ebx
	popl	%eax

	halt

	.stack	0x20
