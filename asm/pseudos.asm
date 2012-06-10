	; pseudo instructions
	.rodata
label:	.dd	$0, $0, $0, $0
	.dd	$1, $2, $3, $4
label2:	.reserve	0x04

	.text
	mrmovl	label, %eax
	rrmovl	%eax, %ecx
	addl	%ecx, %ecx
	rmmovl	%ecx, label2
	pushl	%eax
	pushl	%ecx
	popl	%eax
	popl	%ecx
	halt

	.stack	0x10
