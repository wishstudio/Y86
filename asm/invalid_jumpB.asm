	;another invalid jump
invalid:.dd $1
	.text
	jmp invalid 
	halt
