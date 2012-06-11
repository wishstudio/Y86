	; another combination test
	; expected result: %esi = 5
	.text
	irmovl	mem, %ebx
	mrmovl	(%ebx), %esp
	ret
	halt
rtnpt:  irmovl	$5, %esi
	halt
mem:    .dd	stack
stack:  .dd	rtnpt
