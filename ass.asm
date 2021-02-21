	.data
prompt:	.asciiz "Enter no of points: "
x_coordinate:	.asciiz "Enter x-coordinate of point-"
y_coordinate:	.asciiz "Enter y-coordinate of point-"
colon:	.asciiz " : "
endline:	.asciiz "\n"
output:	.asciiz "Area under the curve formed by given points is: "
overflow:	.asciiz "Overflow occured. Try another set of co-ordinates\n"
twoPtZero:	.float 2.0
Zero:	.float 0.0

	.text
	.globl main
main:

get_input:

	li $v0, 4
	la $a0, prompt
	syscall
	
	li $v0, 5
	syscall
	move $t9, $v0
	
	ble $t9, 0, get_input
	
# $t8 maintains count

	li $t8, 1
	
# $t0 maintains the output area, $t1 stores height of the trapeziums formed, $t2 stores (a+b) i.e. sum of lengths of sides, $t3 stores the product i.e.h(a+b).
# $f0 maintains floating point output, $f3 stores h*(a^2 + b^2), $f2 stores (a+b)

	li $t0, 0
	li $t1, 0
	li $t2, 0
	li $t3, 0
	l.s $f0, Zero
	
# Reads x-coordinate of 1st point:
	
	li $v0, 4
	la $a0, x_coordinate
	syscall
	
	li $v0, 1
	la $a0, ($t8)
	syscall
	
	li $v0, 4
	la $a0, colon
	syscall
	
# Stores the x-coordinate in $t7
	
	li $v0, 5
	syscall
	move $t7, $v0
	
# Reads y-coordinate of 1st point:
	
	li $v0, 4
	la $a0, y_coordinate
	syscall
	
	li $v0, 1
	la $a0, ($t8)
	syscall
	
	li $v0, 4
	la $a0, colon
	syscall

# Stores the y-coordinate in $t6	

	li $v0, 5
	syscall
	move $t6, $v0
	
# Goes to the next point
	
	addi $t8, $t8, 1	

loop:

	bgt $t8, $t9, end_loop
# Reads x-coordinate of ith point:
	
	li $v0, 4
	la $a0, x_coordinate
	syscall
	
	li $v0, 1
	la $a0, ($t8)
	syscall
	
	li $v0, 4
	la $a0, colon
	syscall
	
# Stores the x-coordinate in $t5
	
	li $v0, 5
	syscall
	move $t5, $v0
	
# Reads y-coordinate of ith point:
	
	li $v0, 4
	la $a0, y_coordinate
	syscall
	
	li $v0, 1
	la $a0, ($t8)
	syscall
	
	li $v0, 4
	la $a0, colon
	syscall

# Stores the y-coordinate in $t4	

	li $v0, 5
	syscall
	move $t4, $v0
	
# Checks if both points are on same side or not

	mul $t3, $t4, $t6
	blt $t3, 0, neg_case
	abs $t0, $t4
	abs $t6, $t6

# Finds height and store in $t1; Finds (a+b) and stores in $t2

	sub $t1, $t5, $t7
	add $t2, $t0, $t6
	
# Checks if h*(a+b) overflows or not. If overflows, prompts the user to enter the coordinates again

	mult $t1, $t2
	mfhi $t3
	beq $t3, 0, end_if
	li $v0, 4
	la $a0, overflow
	syscall
	j loop
	
end_if:

# Stores h*(a+b) in $t3

	mflo $t3
	#add $t0, $t0, $t3
	mtc1 $t3, $f4
	cvt.s.w $f4, $f4
	add.s $f0, $f0, $f4
	
	move $t7, $t5
	move $t6, $t4
	
# Goes to the next point
	
	addi $t8, $t8, 1	
	j loop

# Negative Case
	
neg_case:
	abs $t0, $t4
	abs $t6, $t6
	
# Finds h and stores in $t1; Finds (a+b) and stores in $t2
	sub $t1, $t5, $t7
	add $t2, $t0, $t6
	
# Finds (a^2) and stores in $t0. Checks for overflow
	mult $t4, $t4
	mfhi $t0
	beq $t0, 0, end_if_2
	li $v0, 4
	la $a0, overflow
	syscall
	j loop

end_if_2:
		
# Finds b^2 and stores in $t6. Checks for overflow again..Hushhhhh...!!
	mflo $t0
	mult $t6, $t6
	mfhi $t6
	beq $t6, 0, end_if_3
	li $v0, 4
	la $a0, overflow
	syscall
	j loop
	
end_if_3:

# Finds (a^2 + b^2) and stores in $t6
	mflo $t6
	add $t6, $t0, $t6
	
# Finds (a^2 + b^2)*h and stores in $t3...Checks for Overflowwwwwwwww!
	mult $t1, $t6
	mfhi $t3
	beq $t3, 0, end_if_4
	li $v0, 4
	la $a0, overflow
	syscall
	j loop
	
end_if_4:
# Converts $t3 into floating point no. and stores in $f3
	mflo $t3
	mtc1 $t3, $f3
	cvt.s.w $f3, $f3
	
# Convert (a+b) which is stored in $t2 to floating point no. and stores in $f2
	
	mtc1 $t2, $f2
	cvt.s.w $f2, $f2
	
# Finds h*(a^2+b^2)/(a+b) and stores in $f3
	div.s $f3, $f3, $f2
	add.s $f0, $f0, $f3
	
	move $t7, $t5
	move $t6, $t4
	
# Goes to the next point
	
	addi $t8, $t8, 1
	j loop
	
end_loop:

	#mtc1 $t0, $f0
	#cvt.s.w $f0, $f0
	l.s $f1, twoPtZero
	div.s $f1, $f0, $f1
	
	li $v0, 4
	la $a0, output
	syscall

	li $v0, 2
	mov.s $f12, $f1
	syscall
	
	li $v0, 4
	la $a0, endline
	syscall

	li $v0, 10
	syscall
	
	