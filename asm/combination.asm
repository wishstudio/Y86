	; combination test
	; expected result: %eax = 1
	.text
	irmovl	rtnp,%eax
	pushl	%eax
	xorl	%eax,%eax
	jne	target
	irmovl	$1,%eax
	halt
target: ret
	irmovl	$2,%ebx
	halt
rtnp:   irmovl	$3,%edx
	halt
	.stack 0x20
