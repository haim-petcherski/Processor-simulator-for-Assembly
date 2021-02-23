#Quick Sort TEST
    add $sp, $zero, $imm, 1022       #$sp=1022
	add $a0, $zero, $imm, 0          #$a0=0=p, location of the first number in the list
	add $a1, $zero, $imm, 15         #$a1=15=r location of the last number in the list , $a1-$a0=lenthg of list of number to sort
	add $s0, $zero, $imm, 1024       #$s0=1024, pointer to the list *A[15]
	jal $imm, $zero, $zero, qsort    #qsort(A,p,r)
	halt $zero, $zero, $zero, 0      #halt
qsort:
    add $sp, $sp, $imm, -3           # adjust stack for 3 items
	sw $a0, $sp, $imm, 2             # save the first index in the list
	sw $a1, $sp, $imm, 1             #save the last index in the list
	sw $ra, $sp, $imm, 0             # save return address
	blt $imm, $a0, $a1, partition    #if $a0<$a1 go to partition
	beq $imm, $zero, $zero, finish   #jump to finish
partition:
    add $t0, $a0, $imm, -1           #$t2=$a0-1=i
	add $t1, $a1, $imm, 1            #$t1=$a1+1=j
while1:
    lw $t2, $s0, $a0, 0              #$t2=pivot
	add $t1, $t1, $imm, -1           #$t1-=1
	lw $t3, $t1, $s0, 0              # $t3=A[j]
	bgt $imm, $t3, $t2, while1       #repeat on while1 loop until A[j]<=pivot
while2:
    add $t0, $t0, $imm, 1            # $t2+=1
	lw $t3, $s0, $t0, 0              # $t3=A[i]
	blt $imm, $t3, $t2, while2       #repeat on while2 loop until A[i]>=pivot
	blt $imm, $t0, $t1, swap         #if $t2=i<j=$t1 swap between A[j] and A[i]
	add $a1, $t1, $imm, 0            #$a1= $t1= j
	jal $imm, $zero, $zero, qsort    # qsort(A,p,j)
	lw $a1, $sp, $imm, 1             #restore $a1=r
	add $a0, $t1, $imm, 1            #$a0= $t1+1=j+1
	jal $imm, $zero, $zero, qsort    #  qsort(A,j+1,r)
finish:
    lw $ra, $sp, $imm, 0             # restore $ra
	lw $a1, $sp, $imm, 1             # restore $a1
	lw $a0, $sp, $imm, 2             # restore $a0
	add $sp, $sp, $imm, 3            # pop 3 items from stack
	beq $ra, $zero, $zero, 0         # and return
swap:
    lw $t2, $t1, $s0, 0              #$t0=A[j]
	sw $t3, $t1, $s0, 0              #A[j]=A[i]
	sw $t2, $t0, $s0, 0              #A[i]=A[j]
	beq $imm, $zero, $zero, while1   #return to while1 

.word 1024 0x99999999
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
