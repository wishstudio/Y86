	; negative numbers
	.text
	irmovl	$-1, %eax
	irmovl	$1, %ecx
	addl	%ecx, %eax
	halt
