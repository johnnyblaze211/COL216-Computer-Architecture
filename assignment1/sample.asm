.globl main

.text

main: 
	li $t0, 3
	li $t1, -5

	mult $t0, $t1

	mfhi $t8

	mflo $t9

	li $v0, 1
	move $t8, $t0
	syscall

	li $v0, 10
	syscall
	

