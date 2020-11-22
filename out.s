	.data
temp_0:	.word 1
temp_1:	.word 2
temp_2:	.word 0
	.text
	.globl main
main: 	lw $t0, temp_0
	lw $t1, temp_1
	addu $t2, $t0, $t1
	sw $t2, temp_2
exit: 
	li $v0, 10
	syscall
