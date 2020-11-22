	.data
temp_0:	.word 1
temp_1:	.word 2
temp_2:	.word 0
temp_3:	.word 3
temp_4:	.word 0
	.text
	.globl main
main: 	lw $t0, temp_0
	lw $t1, temp_1
	add $t2, $t0, $t1
	sw $t2, temp_2
	lw $t0, temp_2
	lw $t1, temp_3
	add $t2, $t0, $t1
	sw $t2, temp_4
exit: 
	li $v0, 10
	syscall
