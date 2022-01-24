.globl main
.data
	msg: .asciiz "helloo"


.text
	
main:
	li $t0, 0
	li $t1, 1
	li $t2, 2
	li $t3, 3
	li $t4, 4
	li $t5, 5
	li $t6, 6
	li $t7, 7
	li $t8, 8
	li $t9, 9


	li $v0, 1
	la $a0, ($t0)
	syscall

	jal if1

	li $v0, 10
	syscall
	
if1:
	li $v0, 4
	la $a0, msg
	syscall
	jr $ra
	
	

	