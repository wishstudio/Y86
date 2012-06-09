	; divide-by-zero exception test

	.rodata
itable:	.dd	l1, l2, l3, l4, 0, 0, 0, 0

	.text
	lidt	itable
	irmovl	$10, %eax
	irmovl	$0, %ebx
	; divide by zero
	divl	%ebx, %eax
	halt

l1:	irmovl	$1000, %eax
	; memory violation
	mrmovl	(%eax), %ecx
	halt

l2:	; invalid opcode
	.db	$0xF0, $0, $0, $0, $0, $0
	halt

l3:	; invalid register
	; rrmovl %none, %none
	.db	$0x20, $0x88
	halt

l4:	; set idtr to invalid address
	lidt	1000
	; use memory violation to activate double fault
	rmmovl	%eax, l4
	halt
