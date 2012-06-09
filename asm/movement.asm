	; data movement
	.text
	
	irmovl	$0x10, %eax
	irmovl	$0x20, %ebx
	rrmovl	%eax, %ebx
	irmovl	$0x30, %ebx
	rrmovl	%ebx, %esi
	rrmovl	%eax, %ebx
	halt
