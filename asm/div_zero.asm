	; divide-by-zero exception test

	.rodata
itable:	.dd	EXCEP, 0, 0, 0, 0, 0, 0, 0

	.text
	lidt	itable
	irmovl	$10, %eax
	irmovl	$0, %ebx
	divl	%ebx, %eax
	nop
	nop
	nop
	nop
	nop

EXCEP:	nop
	nop
	nop
	nop
	nop
