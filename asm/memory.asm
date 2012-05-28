	; memory data movement
	.data
data0:	.reserve 8
data1:	.reserve 4

	.rodata
data2:	.dd $0x01
data3:	.dd data0, data2

	.text
	irmovl	$0x10, %eax
	rmmovl	%eax, data1

	; test address calculation
	mrmovl	data1, %eax
	irmovl	$1, %ecx
	rmmovl	%eax, data0(%ecx)

	; test memory data forwarding
	irmovl	$2, %ecx
	irmovl	$0x20, %eax
	rmmovl	%eax, data0(%ecx)

	; test memory immediate
	irmovl	data3, %eax
	; fun stuff: indirect addressing
	mrmovl	0(%eax), %ecx
	mrmovl	4(%eax), %edx
