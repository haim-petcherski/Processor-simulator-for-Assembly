 # //DISKTEST2
	jal $imm, $zero, $zero, disktest  # copy sectors 0 - 3 to 4 - 7
	halt $zero, $zero, $zero, 0       # halt
disktest:
	add $t0, $zero, $imm, 0           # $t0 = 0
	add $t1, $zero, $imm, 1           # $t1 = 1
	add $t2, $zero, $imm, 1024          # $t0 = 1024
	out $t2, $zero, $imm, 16          #setting diskbuffer in line 1024
L1:
	out $t0, $imm, $zero, 15          # set disksector
	out $t1 $zero, $imm, 14           # set diskcmd as $t1
L2:
	in $t2, $zero, $imm, 17           # $t2=diskstatus
	bne $imm, $t2, $zero, L2          # run in loop until diskstatus=0
	out $zero, $imm, $zero, 4         # turn off irq1status
L3 :
	add $t3, $zero, $imm, 1           # $t2 = 1
	bne $imm, $t1, $t3, L4            # if last discmd is reading go to L4
	add $t1, $imm, $zero, 2           # $t1 = 2, #if last diskcmd was to read the code will get to here so next diskcmd will be to write
	add $t0, $t0, $imm, 4             # $t0 += 4
	beq $imm, $zero, $zero, L1        # go t0 L1
L4 :                                  #if last diskcmd was to write the code will get to here so next diskcmd will be to read
	add $t1, $t3, $zero, 0            # $t1 = $t2 = 1
	sub $t0, $t0, $imm, 3             # $t0-=3
	beq $ra, $imm, $t0, 4             # if  need to read sector number 4 finish
	beq $imm, $zero, $zero, L1        # go to L1