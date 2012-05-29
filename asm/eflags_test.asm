	; test eflags
	.text
	irmovl	$0x7FFFFFFF, %eax
	rrmovl	%eax, %ebx
	addl	%ebx, %eax
	; expected result:
	; SF, OF are set

	irmovl	$10, %eax
	irmovl	$9, %ebx
	subl	%ebx, %eax
	; expected result:
	; CF is set

	irmovl	$7, %eax
	rrmovl	%eax, %ebx
	cmpl	%ebx, %eax
	; expected result
	; CF, ZF are set
