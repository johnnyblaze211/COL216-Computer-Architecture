.globl main
.text

main:
	
	li $t1, 0 #counter
	li $t2, 0 #..
	lwc1 $f4, zeroFloat #sum

	li $v0, 5
	syscall

	move $t5, $v0

loop:
	beq $t1, $t5, end
	
	li $v0, 6
	syscall

	add.s $f4, $f4, $f0
	addi $t1, $t1, 1
	j loop
	
	
	
end:

	mov.s $f12, $f4
	li $v0, 2
	syscall
	
	li $v0, 10
	syscall
	
	

.data
msg:	.asciiz "Hello World!"
zeroFloat: .float 0.0

