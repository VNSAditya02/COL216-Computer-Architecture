	.data
prompt: .asciiz "Enter the length of the string: "
postfix: .asciiz "Enter the postfix experiment: "
endline: .asciiz "\n"
userAns: .space 1000
wrong_char: .asciiz "You entered a character that is not permitted (or) The length of the string entered is wrong\n"
invalid_exp: .asciiz "The postfix expression is invalid (or) The length of the string entered is wrong\n"
over_flow: .asciiz "Overflow occured\n"
result: .asciiz "Result of the expression is: "

	.text
	.globl main	
main:

get_input_length:

	li $v0, 4
	la $a0, prompt
	syscall
	
# $t0 stores the length of the input string

	li $v0, 5
	syscall
	move $t0, $v0
	
	ble $t0, 0, get_input_length 
	addi $t0, $t0, 2
	
# Reads the input string from the userAns

	li $v0, 4
	la $a0, postfix
	syscall
	
	li $v0, 8
	la $a0, userAns
	la $a1, ($t0)
	syscall
	
# Start address of the string is stores in $s0

	la $a1, userAns
	
# $t1 is counter.
	
	li $t1, 1

# Earlier added 2 to $t0, so subtracted now
	sub $t0, $t0, 2
	
# $t7 maintains the bytes used upto a particular instant.
	
loop:

	bgt $t1, $t0, end_loop
	
# $s1 stores the ascii value of the present character
	lb $s1, 0($a1)
	
	beq $s1, 43, plus
	
	beq $s1, 45, minus
	
	beq $s1, 42, product
	
	blt $s1, 48, error
	
	bgt $s1, 57, error
	
	j digit

# $t3 gives the last digit added to the stack
# $t4 stores the last-but-one digit added to the stack
# $t5 stores $t3 (operator) $t4
# If no of bytes used is less than 8 i.e. the no. of digits present on stact is less than 2, then the postix expression is invalid.

plus:

	blt $t7, 8, invalid
	lw $t3, ($sp)
	lw $t4, 4($sp)
	add $t5, $t3, $t4
	addi $sp, $sp, 8
	addi $sp, $sp, -4
	sw $t5, ($sp)
	li $t8, 4
	sub $t7, $t7, $t8
	j end_if_1
	
minus:

	blt $t7, 8, invalid
	lw $t3, ($sp)
	lw $t4, 4($sp)
	sub $t5, $t4, $t3
	addi $sp, $sp, 8
	addi $sp, $sp, -4
	sw $t5, ($sp)
	li $t8, 4
	sub $t7, $t7, $t8
	j end_if_1

product:

	blt $t7, 8, invalid
	lw $t3, ($sp)
	lw $t4, 4($sp)
	mult $t3, $t4
	mfhi $t5
	bne $t5, 0, overflow
	mflo $t5
	addi $sp, $sp, 8
	addi $sp, $sp, -4
	sw $t5, ($sp)
	li $t8, 4
	sub $t7, $t7, $t8
	j end_if_1
	

digit:

	addi $sp, $sp, -4
	
# $t2 stores the value of the present character

	sub $t2, $s1, 48
	addi $t7, $t7, 4
	sw $t2, ($sp)

end_if_1:

	addi $t1, $t1, 1
	addi $a1, $a1, 1
	j loop

# $t6 stores the final result	
	
end_loop:

	bne $t7, 4, invalid

	li $v0, 4
	la $a0, result
	syscall
	
	lw $t6, ($sp)
	li $v0, 1
	la $a0, ($t6)
	syscall

	li $v0, 4
	la $a0, endline
	syscall	
	
	j end_code

error:

	li $v0, 4
	la $a0, wrong_char
	syscall
	
	j end_code

invalid:
	li $v0, 4
	la $a0, invalid_exp
	syscall
	j end_code
	
overflow:
	li $v0, 4
	la $a0, over_flow
	syscall
	
end_code:

	li $v0, 10
	syscall
	
	

