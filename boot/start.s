.global _reset
_reset:
	# Set up stack pointer
	LDR X2, =stack_top
	MOV SP, X2
	# Magic number for debug with qemu-system-aarch64
	MOV X13, #0x1111
	
.global main
	bl	main
	# shoud never reach here, but just in case, Loop endlessly
loop:
	MOV X13, #0x7777
    b   loop