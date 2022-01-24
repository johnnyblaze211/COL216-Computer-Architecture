.globl main
.text

.data
	stack: .space 100
	input: .space 100
	print_exp_msg: .asciiz "enter expression\n"
	non_valid_token_msg: .asciiz "\nInputError: Please enter a valid digit or operator"
	input_error_msg:	.asciiz "\nWrong Input"


.text
	

main:	
	li $v0, 4
	la $a0, print_exp_msg
	syscall

	li $v0, 8
	la $a0, input
	li $a1, 100
	syscall

	j inToPost



inToPost:
	li $t0, 0	#counter
	li $t1, 0	#stack counter
	addi $t4, $t1, -1
	j while1


## while loop 1
while1:
	lb $s0, input($t0)	#character input
	beq $s0, $0, EOwhile1	#end while loop if input[i] == '\0'
	
	li $t2, 48
	slt $t2, $s0, $t2

	li $t3, 57
	sgt $t3, $s0, $t3

	or $t2, $t2, $t3
	
	beq $t2, $0, isOperandTrue
	j isOperandFalse


isOperandTrue:
	li $v0, 11
	la $a0, ($s0)
	syscall
	j isOperandDone


isOperandFalse:
	li $t2, 43
	beq $s0, $t2, isOperatorTrue
	li $t2, 45
	beq $s0, $t2, isOperatorTrue
	li $t2, 42
	beq $s0, $t2, isOperatorTrue
	li $t2, 47
	beq $s0, $t2, isOperatorTrue
	li $t2, 37
	beq $s0, $t2, isOperatorTrue
	li $t2, 40
	beq $s0, $t2, isOperatorTrue
	

	j isOperatorFalse


isOperatorTrue:

	addi $t4, $t1, -1
	li $t2, 0
	li $t3, 0

	beq $t1, $0, isOperatorTrueif1True
	
	lb $s1, stack($t4)	#stores stack.top()
	li $t2, 43
	seq $t3, $t2, $s0	#t3 is one if input[i] = '+'
	li $t2, 45
	seq $t3, $t2, $s0	#t3 is one if input[i] = '-'

	li $t2, 42
	beq $s0, $t2, t3to3	#case '*'
	li $t2, 47
	beq $s0, $t2, t3to3	#case '%'
	li $t2, 37
	beq $s0, $t2, t3to3	#case '/'
	

	li $t2, 40
	beq $s0, $t2, t3to100	#case '('
	
	
t3to3:
	li $t3, 3
	j t3_done

t3to100:
	li $t3, 100
	j t3_done


#checking inPrec(s.top())
t3_done:
	li $t9, 0		#Stores inPrec(s.top())
	
	li $t2, 43
	beq $t2, $s1, t9to2
	li $t2, 45
	beq $t2, $s1, t9to2

	li $t2, 42
	beq $s1, $t2, t9to4
	li $t2, 47
	beq $s1, $t2, t9to4
	li $t2, 37
	beq $s1, $t2, t9to4
	
	li $t2, 40
	beq $s1, $t2, t9to0

t9to2:
	li $t9, 2
	j t9_done

t9to4:
	li $t9, 4
	j t9_done

t9to0:
	li $t9, 0
	j t9_done

t9_done:
	bgt $t3, $t9, isOperatorTrueif1True
	j isOperatorTrueif1False





isOperatorTrueif1True:
	sb $s0, stack($t1)
	addi $t1, $t1, 1
	addi $t4, $t1, -1
	j isOperandDone



isOperatorTrueif1False:
	j isOperatorTrueif1Falsewhile


isOperatorTrueif1Falsewhile:
	beq $t1, $0, isOperatorTrueif1Falsewhiledone
	lb $s0, stack($t4)
	
	li $t2,0
	li $t3,0
	li $t9,0
	
	li $t2, 43
	seq $t3, $t2, $s0	#t3 is one if input[i] = '+'
	li $t2, 45
	seq $t3, $t2, $s0	#t3 is one if input[i] = '-'

	li $t2, 42
	beq $s0, $t2, t3to3b	#case '*'
	li $t2, 47
	beq $s0, $t2, t3to3b	#case '%'
	li $t2, 37
	beq $s0, $t2, t3to3b	#case '/'
	

	li $t2, 40
	beq $s0, $t2, t3to100b	#case '('



t3to3b:
	li $t3, 3
	j t3done2

t3to100b:
	li $t3, 100
	j t3done2

t3done2:
	li $t2, 43
	beq $t2, $s1, t9to2b
	li $t2, 45
	beq $t2, $s1, t9to2b

	li $t2, 42
	beq $s1, $t2, t9to4b
	li $t2, 47
	beq $s1, $t2, t9to4b
	li $t2, 37
	beq $s1, $t2, t9to4b
	
	li $t2, 40
	beq $s1, $t2, t9to0b

t9to2b:
	li $t9, 2
	j t9done2

t9to4b:
	li $t9, 4
	j t9done2

t9to0b:
	li $t9, 0
	j t9done2

t9done2:
	bge $t3, $t9, isOperatorTrueif1Falsewhiledone
	li $v0, 11
	la $a0, stack($t4)
	syscall
	sb $0, stack($t4)
	addi $t1, $t1, -1
	addi $t4, $t1, -1
	j isOperatorTrueif1Falsewhile
	

	
isOperatorTrueif1Falsewhiledone:
	sb $s0, stack($t1)
	addi $t1, $t1, 1
	addi $t4, $t1, -1
	j isOperandDone
	

isOperatorFalse:
	li $t2, 41
	bne $s0, $t2, nonValidTokenError

	j while3
	
	



while3:
	li $t2, 40
	beq $t1, $0, wronginput
	lb $s1, stack($t4)
	beq $s1, $t2, while3done
	
	li $v0, 11
	la $a0, stack($t4)
	syscall
	
	sb $0, stack($t4)
	addi $t1, $t1, -1
	addi $t4, $t1, -1

	beq $t0, $0, wronginput
	j while3
	


while3done:
	sb $0, stack($t4)
	addi $t1, $t1, -1
	addi $t4, $t1, -1
	j isOperandDone


wronginput:
	li $v0, 4
	la $a0, input_error_msg
	syscall
	j end
	


nonValidTokenError:
	li $v0, 4
	la $a0, non_valid_token_msg
	syscall
	j end
	
	
isOperandDone:
	j while1_update
	

while1_update:
	addi $t0, $t0, 1
	j while1


EOwhile1:
	j while4


while4:
	beq $t1, $0, end
	lb $s1, stack($t4)
	li $t2, 40
	
	beq $s1, $t2, wronginput
	
	li $v0, 11
	la $a0, stack($t4)
	syscall

	sb $0, stack($t4)
	addi $t1, $t1, -1
	addi $t4, $t1, -1
	
	j while4
	
	


end:
	li $v0, 10
	syscall



	


	