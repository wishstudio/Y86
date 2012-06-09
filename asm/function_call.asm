	; function call

	.text
	irmovl	$10, %eax
	call	func
	halt

func:	irmovl	$20, %ebx
	addl	%ebx, %eax
	ret

	.stack	0x20
