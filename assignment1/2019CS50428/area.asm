.globl main
.text

.data
	zeroFloat: 		.float 0.0
	input_n_msg:		.asciiz "Enter the no. of points: n = "
	input_x:			.asciiz "Enter X coordinate: x = "
	input_y:			.asciiz "Enter Y coordinate: y = "
	print_sum_msg:		.asciiz "Area enclosed by points is: "
	input_x_error_msg:	.asciiz "\nInputError: X coordinates must be given in strictly increasing order"
	input_error_msg:		.asciiz "\nInputError: Please enter a positive integer"
	overflow_error_msg: 	.asciiz "\nOverflowError: Result exceeded 32 bits"
	halfFloat: 		.float 0.50

.text


main:
	#input variable declarations
	
	li $t1, 0				# counter
	li $t2, 0				# sum
	lwc1 $f4 zeroFloat			# zero floating point variable

	li $v0, 4
	la $a0, input_n_msg
	syscall

	
	li $v0, 5				# syscall code 5 for taking integer input
	syscall					# taking input n (no. of points)
	
	move $t3, $v0				# storing n in t3
	ble $t3, $t1, input_n_error		# input error if n is 0 or negative

	li $v0, 4
	la $a0, input_x
	syscall
	
	li $v0, 5
	syscall		
	
	move $t4, $v0				# taking 1st X input and storing it in t4

	li $v0, 4
	la $a0 input_y
	syscall
	
	li $v0, 5
	syscall
	
	move $t5, $v0				# taking 1st Y input and storing it in t5

	addi $t1, $t1, 1

	
	
	j loop
	
	
	
	
## Displays error message when input n is negative or zero
input_n_error:

	li $v0, 4				# syscall for string output
	la $a0, input_error_msg			# error message
	syscall

	j end					# jumps to end


	
## loop construct
loop:
	beq $t1, $t3, print_sum			# if(counter == n), print_sum();

	li $v0, 4
	la $a0, input_x
	syscall
	
	li $v0, 5				
	syscall		
	
	move $t6, $v0				# X coordinate of 2nd point in stored in t6

	li $v0, 4
	la $a0, input_y
	syscall
	
	li $v0, 5
	syscall
	
	move $t7, $v0				# Y coordinate of 2nd point in buffer in t7
	
	ble $t6, $t4, input_x_error		# if (x2 < x1) input_x_error();

	li $t8, 0				
	add $t8, $t7, $t5			# y1 + y2 stored in t8
	li $t9, 0
	sub $t9, $t6, $t4			# x2 - x1 stored in t2

	mult $t8, $t9				# HI and LO registers store (y1+y2)*(x1-x2)
	mflo $t9					# t9 now stores LO register's value (32 LSBs of multiplication)
	mfhi $t8					# t8 stores HI register's value (32 MSBs of multiplication)

	li $t0, -1
	seq $a1, $t8, $t0
	slt $a2, $t9, $0
	and $a3, $a1, $a2

	move $s1, $t8				# this portion of code basically checks if overflow is there in HI register

	seq $s2, $s1, $0				# (if (value in LO is negative and in HI is -1) or if (value in HI is 0)) --> no_overflow
	or $s3, $s2, $a3
	
	beq $s3, $0, overflow_error		# exit loop if overflow condition is satisfied
	

#for no overflow condition to occur during multiplication, product should not exceed 32 bits
#Therefore, the HI register must be zero (or -1 for negative integers)to ensure bits have not overflown
	



	li $t8, 0
	add $t8, $0, $t2				# t8 temporarily stores value of t2 before update		
	add $t2, $t2, $t9			# t2 is updated with the new sum t2 = t2 + (y1+y2)*(x2-x1)
	

# t2 --> new_sum, t8 --> prev_sum, t9 --> (y1 + y2) * (x2 - x1)

# a1, a2, a3 can attain values 1 or 0.
# a1 is 1 if t8 is non-negative and 0 otherwise. Similarly for a2, a3.


	slt $a1, $0, $t8				
	slt $a2, $0, $t9
	slt $a3, $0, $t2


# Since, valOf($t2) = valOf($t9) + valOf($t8)
	
# for overflow to not occur while calculating sum, t8 and t9 must be of opposite sign
# OR t8, t9, t2 all must have the same sign 


	xor $a1, $a1, $a2			# xor is 1 if a1 and a2 are unequal i.e valOf($t9) != valOf($t8)
	bne $a1, $0, no_overflow			# if(a1!=0) no_overflow();
	beq $a3, $a2, no_overflow			# for remaining case, if terms have same sign, no overflow.
	j overflow_error


## error when x input is not given in sorted order
input_x_error:
	li $v0, 4
	la $a0, input_x_error_msg
	syscall
	j end



## if no_overflow, loop continues	
no_overflow:
	move $t4, $t6				
	move $t5, $t7				# previous values of x2, y2 are pushed into registers storing x1, y1
	addi $t1, $t1, 1				# update counter
	j loop					
	

## output overflow error message and exit	
overflow_error:
	li $v0, 4
	la $a0, overflow_error_msg
	syscall	
	j end


## to print final value of area
print_sum:
	mtc1 $t2, $f4				# final sum integer value stored in floating point register f4
	cvt.s.w $f4, $f4				# convert word to floating point in f4
	lwc1 $f6, halfFloat			# stores value 0.5 in another register
	mul.s $f12, $f6, $f4			# multiplies sum by 0.5 to get final answer

	li $v0, 4
	la $a0, print_sum_msg
	syscall

	li $v0, 2				
	syscall
	j end					#syscall code 2 to print floating point value to console
	

end:
	li $v0, 10				#syscall code 10 to exit
	syscall
	
	