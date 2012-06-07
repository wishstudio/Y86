	; function call

	.text
	irmovl	$10, %eax
	call	func
	nop
	nop
	nop
	nop
	nop

func:	irmovl $20, %ebx
	addl	%ebx, %eax
	ret
