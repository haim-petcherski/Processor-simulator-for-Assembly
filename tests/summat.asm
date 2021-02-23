#test of sum of matrices 
    add $a0, $zero, $imm, 0x100       #$a0=0x100, adress of first matrice
	jal $imm, $zero, $imm, summat     #sum two matrices into third matrice
	halt $zero, $zero, $zero, 0       #halt
summat:
    add $t0, $zero, $a0, 0            #$t0=$a0
for:
    lw $t1, $t0, $zero, 0             # $t1 is a number in first matrix
	lw $t2, $t0, $imm, 16             # $t2 is a number in second matrice in the same relative place as $t1
	add $t3, $t2, $t1, 0              # $t3=$t1+$t2
	sw $t3, $t0, $imm, 32             # store $t3 in third matrice in same relative place as $t1 and $t2
	add $t0, $t0, $imm, 1  #$t0+=1    #continue to the next numbers in the matrices
	beq $ra, $t0, $imm, 0x110         # if $t0=0x110 finish to some and return
	beq $imm, $zero, $zero, for       # jump back to the loop for and continue to sum

.word 0x100 4
.word 0x101 4
.word 0x102 5
.word 0x103 3
.word 0x104 32
.word 0x105 11
.word 0x106 33
.word 0x107 4
.word 0x108 45
.word 0x109 53
.word 0x10a 31
.word 0x10b 577
.word 0x10C 67
.word 0x10D 54
.word 0x10E 677
.word 0x10f 12
.word 0x110 34
.word 0x111 45
.word 0x112 56
.word 0x113 63
.word 0x114 42
.word 0x115 45
.word 0x116 56
.word 0x117 65
.word 0x118 43
.word 0x119 56
.word 0x11a 43
.word 0x11b 89
.word 0x11c -50
.word 0x11d 444
.word 0x11e 236
.word 0x11f 66