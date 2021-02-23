	#/*clock2*/
	.word 1024 0x195955                  # saving srarting time
	.word 1025 0x1fffff                  # saving important number to play afterwards the time 20:00:00
	jal $imm, $zero, $zero, clock        # run the clock for 10 seconds from 19:59:55 to 20:00:05
	halt $zero, $zero, $zero, 0          # halt
clock:
	add $t0, $zero, $imm, 255            # $t0 = 255
	out $t0, $zero, $imm, 13             # timermax = 255
	add $t0, $zero, $imm, 1              # $t0 = 1
	lw $t1, $zero, $imm, 1024            # enter the starting time from adress 1024 into $t0
	out $t1, $zero, $imm, 10             # start to display the time
	out $t0, $zero, $imm, 11             # turn on timerenable
L1:
	in $t0, $zero, $imm, 3               # read irq0status into $t1
	beq $imm, $t0, $zero, L1             # if irq0status eqyal 1 go out from loop L1
L2 :
	out $zero, $zero, $imm, 3            # turn off irq0status
	add $t1, $t1, $imm, 1                # $t1 += 1
	out $t1, $zero, $imm, 10             # display the new time after 1 second
	add $t2, $zero, $imm, 9              #$t2=9
	and $t3, $imm, $t1, 0xf              # checking if the 4 LSB bits equal to 9
	beq $imm, $t3, $t2, L3               # if the current time is 195959 go to L3
	add $t2, $zero, $imm, 5              #$t2=5     
	and $t3, $t1, $imm, 0xf              # checking if the 4 LSB bits equal to 5
	beq $ra, $t3, $t2, 0                 # if the crrent time is 200005 exit
	beq $imm, $zero, $zero, L1           #go back to loop L1
L3 :
	lw $t1, $zero, $imm, 1025            # enter important number from adress 1025 into $t1
	beq $imm, $zero, $zero, L1           #go back to loop L1 