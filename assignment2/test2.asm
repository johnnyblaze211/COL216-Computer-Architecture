.globl main
.data
	input: .space 110
	print_exp_msg: .asciiz "enter expression\n"
	character_msg: .asciiz "\ncharacter is: "
	yay:	.asciiz "\nyay"

.text
	

main:
	li $v0, 4
	la $a0, print_exp_msg
	syscall

	li $t0,0

	li $v0, 8
	la $a0, input
	li $a1, 100
	syscall

	lb $t3, input($t0)
	addi $t0, $t0, 1
	lb $t1, input($t0)
	addi $t0, $t0, 1
	lb $t2, input($t0)

	li $v0, 4
	la $a0, character_msg
	syscall
	
	li $t9, 40
	beq $s0, $t9, end

	li $v0, 4
	la $a0, yay
	syscall

	j end
	
	

	





end:
	li $v0, 10
	syscall