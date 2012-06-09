	; Forwarding Priority
	.text
	irmovl	$10, %edx
	irmovl	$3, %edx
	rrmovl	%edx, %eax
	halt
