	.data
temp_0:	.word 1
temp_1:	.word 2
temp_2:	.word 0

	.text
	.globl main
main:
				#temp_2 = temp_0 + temp_1
	lw $t0, temp_0
	lw $t1, temp_1
	add $t2, $t0, $t1
	sw $t2, temp_2
				#print integer temp_2
	lw $a0, temp_2
	li $v0, 1
	syscall

exit:
	li $v0, 10
	syscall
