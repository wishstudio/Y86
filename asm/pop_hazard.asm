	; popl hazard test
	.text

	irmovl	$10, %eax
	irmovl	$3, %ecx
	pushl	%eax
	popl	%edx
	rrmovl	%edx, %ecx

	.stack	0x20
