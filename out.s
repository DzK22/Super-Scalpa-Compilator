	.data
temp_0:	.word 5
temp_1:	.word 4
temp_2:	.word 0
temp_3:	.word 1
temp_4:	.word 2
temp_5:	.word 0
temp_6:	.word 5
temp_7:	.word 3
temp_8:	.word 0
temp_9:	.word 5
temp_10:	.word 10
temp_11:	.word 0
	.text
	.globl main
main: 				#temp_11 = temp_9 - temp_10
	lw $t0, temp_9
	lw $t1, temp_10
	sub $t2, $t0, $t1
	sw $t2, temp_11
				#print integer temp_11
	lw $a0, temp_11
	li $v0, 1
	syscall
exit: 
	li $v0, 10
	syscall
