#bubble sort test
    add $a0, $zero, $imm, 1024       # $a0=1024, adress where the list  starting &A
    add $a1, $zero, $imm, 15         # $a1=16=N, N is the lenght of the list
    jal $imm, $zero, $zero, bubble   #jump to calculate bubblesort(*A,16)
    halt $zero, $zero, $zero, 0      #halt
bubble:
    add $t0, $zero, $zero, 0         #$t0=0=i
    for1:
	add $t1, $a0, $zero, 0           #$t1=&A+j
for2:
    lw $t2, $t1, $zero, 0            #$t2= A[j]
	lw $t3, $t1, $imm, 1             #t3=A[j+1]
	bgt $imm, $t2, $t3, swap         #if A[j]>A[j+1] jump to swap
continue:
    add $t1, $t1, $imm, 1            #$t1+=1=&A+j+1
	sub $t2, $a1, $imm, 0            #$t2= 16-1=15
	sub $t3, $t2, $t0, 0             #$t3=16-1-i
	add $t3, $t3, $a0, 0             #$t3=*A+16-1-i
	blt $imm, $t1, $t3, for2         # if $t1<$t3 repeat loop for2
	add $t0, $t0, $imm, 1            # $t0+=1=i+1
	blt $imm, $t0, $t2, for1         #if $t0<$t2=16-1=N-1 repeat for1
	beq $ra, $zero, $zero, 0         # retun
swap:
    sw $t3, $zero, $t1, 0            #A[j]=A[j+1]
	sw $t2, $imm, $t1, 1             #A[j+1]=A[j]
	beq $imm, $zero, $zero, continue #jump back to "continue" label

.word 1024 20
.word 1025 7
.word 1026 -5
.word 1027 6
.word 1028 13
.word 1029 8
.word 1030 -5
.word 1031 100
.word 1032 100
.word 1033 -39993
.word 1034 32
.word 1035 -25
.word 1036 0
.word 1037 34
.word 1038 51
.word 1039 0x3426