.globl main
.data

.text
	

main: 
	addi $t0, $t0, 7 
	addi $t1, $t1, 13
	add $t2, $t1, $t0
	beq $t2, $0, branch
	


branch:
	mul $t3, $t1, $t0
	j branch2

branch2:
	addi $t4, $t4, 10
	bne $t4, $0, end

end:
	li $v0, 10
	syscall
	
	
