#leds test
	jal $imm, $zero, $zero, leds         # turn on the leds one by one
	halt $zero, $zero, $zero, 0          # halt
leds :
	add $t0, $zero, $imm, 255            # $t1 = 255
	out $t0, $zero, $imm, 13             # timermax = 255
	add $t0, $zero, $imm, 1              # $t0 = 1
	out $t0, $zero, $imm, 9              # turn on the LSB led
	out $t0, $zero, $imm, 11             # turn on timerenable
L1 :
    in $t1, $zero, $imm, 3               # stay in the loop for 1 second 
	beq $imm, $zero, $t1, L1             # if irq0status turn on get out of loop
L2:
    out $zero, $imm, $zero, 3            # turn off irq0status
	sll $t0, $t0, $imm, 1                # $t0*=2
	out $t0, $zero, $imm, 9              # turn on the next led and turn on the one before
	beq $ra, $zero, $t0, 0               # if all the leds turned off exit
	add $zero, $zero, $zero, 0           # a line to get a symetry of 256 cycles between one turn on of a led to another one
	beq $imm, $zero, $zero, L1           # go back to the loop for 1 second 