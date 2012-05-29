	; jump test

	.text
	; test jmp
	irmovl	$1, %eax
	jmp	l1
	irmovl	$2, %eax
l1:	nop
	; %eax == $1

	; test jcc
	irmovl	$10, %eax
	irmovl	$20, %ebx
	cmpl	%ebx, %eax
	jg	bad

	irmovl	$10, %eax
	irmovl	$20, %ebx
	cmpl	%ebx, %eax
	jl	l2
	jmp	bad

l2:	cmpl	%eax, %eax
	je	l3
	jmp	bad

l3:	nop ; good

bad:	; nothing
