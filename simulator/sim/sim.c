#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)
#define MAX_LEN_OF_FILE 4096
#define MAX_LINE 500
#define NUM_SECTORS 128
#define NUM_WORDS_IN_SECTOR 128
#define MAX_LINE 500

//ALL THE VARIABLES ARE GLOBAL SO THE FUNCTIONS WONT NEED TO CALL TOO MANY VARIABLES
FILE *memin = NULL, *memout = NULL, *trace = NULL, *regout = NULL, *diskin = NULL, *irq2in = NULL, *hwregtrace = NULL, *cycles = NULL, *leds = NULL, *diskout = NULL, *display = NULL;
int R[16] = { 0 }, mem[MAX_LEN_OF_FILE] = { 0 }, diskinmem[NUM_SECTORS][NUM_WORDS_IN_SECTOR] = { 0 }, command = 0, irq = 0, irq2[MAX_LEN_OF_FILE] = { 0 }, rd = 0, rs = 0, rt = 0, opcode = 0, imm = 0, finalHalt = 0, CurrCycleOfStartDIsk = 0, Irq2Follower = 0;
bool inIrq = false, jumpedNow = false;
unsigned int  IOR[18] = { 0 }, PC = 0;
unsigned long TotalCycles = 0;


void PrintToMemout();//print array mem to memout.txt
void PrintToCycles();//print TotalCycles to cycles.txt
void PrintToTrace(int PC);//prints info if every cycle to trace.txt
void PrintToDiskOut();//prints matrix diskinmem to diskout.txt
void PrintReg();//prints the registers to regout.txt
void PrintDisplay(unsigned int Displaynum);//prints the required info to Display.txt
void PrintOUTHwregtrace(int numOfReg);//print the data to hwregout.txt for out(WRITE) function
void PrintINHwregtrace(int numOfReg);//print the data to hwregout.txt for in(IN) function
void PrintLeds();//Print the info about the leds to leds.txt

void ReadDiskIn();//reads all diskin to matrix diskinmem[]
void ReadIrq2Cycles();//saves the cycles of irq2 into array irq2[]
void ReadMemory();//Reads all memin to mem[]

void ClearTabSpace(char Hex[]); //takes out from str all the Tabs and spaces
int SignExtentionForImm(int Num);//Value of Complement Number
int ShiftRightLogic(int Num, int ShiftAmount);//Shift Right Logic

void IN();//IN command, takes care to insert the required value from IO reg to normal reg and calls the func PrintINHwregtrace()
void OUT();//OUT command,  takes care to insert the required value from normal regto IO reg and calls the func PrintOUTHwregtrace()
void Action();//Reads the Opcode and detects what command we need to do, and does it.
void CheckEndOfWriteToDisk();//Checks If disk finished his write/read command, and if not ready adds to the cycle Busy counter +1 so we will know when we reach 1024
void AddToClock();//adds 1 to the clock or reset it
void AddToTimer();//add time to timer if enabled, or reset it, and takes care of interrupt status

void CloseAllFiles();//closes all the files




int main(int argc, char* argv[])
{
	if ((irq2in = fopen(argv[3], "r")) == NULL)//open irq2in
	{
		printf("Could Not Open irq2in File");
		CloseAllFiles();
		return 1;
	}
	if ((memin = fopen(argv[1], "r")) == NULL)//open memin
	{
		printf("Could Not Open memin File");
		CloseAllFiles();
		return 1;
	}
	if ((display = fopen(argv[10], "w")) == NULL)//open memout
	{
		printf("Could Not Open memout File");
		CloseAllFiles();
		return 1;
	}
	if ((leds = fopen(argv[9], "w")) == NULL)//open memout
	{
		printf("Could Not Open memout File");
		CloseAllFiles();
		return 1;
	}
	if ((hwregtrace = fopen(argv[7], "w")) == NULL)//open memout
	{
		printf("Could Not Open memout File");
		CloseAllFiles();
		return 1;
	}
	if ((trace = fopen(argv[6], "w")) == NULL)//open trace
	{
		printf("Could Not Open trace File");
		CloseAllFiles();
		return 1;
	}
	if ((diskin = fopen(argv[2], "r")) == NULL)// open diskin
	{
		printf("Could Not Open diskin File");
		CloseAllFiles();
		return 1;
	}
	ReadMemory();//saves all memory in mem[]
	ReadDiskIn();//saves all Disk in diskinmem[]
	ReadIrq2Cycles();//saves all Cycles of irq2 from irq2in
	fclose(memin); //close files
	fclose(irq2in);//close irq2in                                   
	fclose(diskin);//close diskin

	while (finalHalt == 0 && PC <= MAX_LEN_OF_FILE - 1)//start program
	{
		jumpedNow = false; //reset to false, jumpednow Tells us if the command changes the PC by sort of jump, if not so we need to PC++, But if we jumped to another PC we dont need to PC++
		irq = (IOR[0] & IOR[3]) | (IOR[1] & IOR[4]) | (IOR[2] & IOR[5]);//checks if there is interrupt
		if (irq == 1 && inIrq == false)//checks if we need to enter new interrupt or if we are in a middle of one
		{
			IOR[7] = PC;//restores PC in irqreturn
			PC = IOR[6];//takes irqhandler to PC
			inIrq = true;// says that we are in a middle of interrupt
		}
		if (irq2[Irq2Follower] == TotalCycles)//if we need to turn on irq2status
		{
			IOR[5] = 1;//irq2status=1
			Irq2Follower++;//+1 to the index of irq2[] so we can get the next cycle for irq2 interrupt
		}
		Action();//calls the Func that deals with the command
		CheckEndOfWriteToDisk();//Checks If disk finished his write/read command and adds one to the cycles of the commad if nessesary
		AddToClock();//add 1 to clock or reset it
		AddToTimer(); //add time to timer
		if (jumpedNow == false)//checks if we used branch or j functions now or not
			PC++;
		TotalCycles += 1; // add 1 to the cycle
		if (finalHalt == 1)//if Halt was called
			break;


	}
	if ((cycles = fopen(argv[8], "w")) == NULL)//open Cycles
	{
		printf("Could Not Open Cycles File");
		CloseAllFiles();
		return 1;
	}
	if ((memout = fopen(argv[4], "w")) == NULL)//open memout
	{
		printf("Could Not Open memout File");
		CloseAllFiles();
		return 1;
	}
	if ((regout = fopen(argv[5], "w")) == NULL)//open memout
	{
		printf("Could Not Open memout File");
		CloseAllFiles();
		return 1;
	}
	if ((diskout = fopen(argv[11], "w")) == NULL)//open memout
	{
		printf("Could Not Open memout File");
		CloseAllFiles();
		return 1;
	}
	PrintToCycles();//print TotalCycles to cycles
	PrintToMemout();//print mem to memout
	PrintReg();//print R[] to regout
	PrintToDiskOut();//print diskinmem to diskout

	CloseAllFiles();//close all files
}

void ReadDiskIn() //Read all diskin to a matrix of values
{

	char Fpointer[MAX_LINE] = { 0 };
	int i = 0, j = 0;
	while (1)//did this because we dont know the size of the file, and we want to exit when we reach EOF
	{
		if (feof(diskin))//if we reached EOF
		{
			break;
		}
		fgets(Fpointer, MAX_LINE, diskin);//read line
		ClearTabSpace(Fpointer);//clear tabs and spaces
		diskinmem[i][j] = (int)strtoul(Fpointer, NULL, 16);//convert str Hex to int
		if (j == NUM_SECTORS - 1 && i < NUM_SECTORS - 1) //if line is finished but not the last line of the matrix
		{
			j = 0;
			i++;
		}
		else if (j == NUM_SECTORS - 1 && i == NUM_SECTORS - 1) //if reaches end of matrix diskinmem[][]
			break;
		else // if we are in a middle of a row
			j++;

	}

}
void ReadIrq2Cycles()//saves the cycles of irq2 from the file irq2in into an array
{
	char Fpointer[MAX_LINE] = { 0 };
	for (int i = 0; fgets(Fpointer, MAX_LINE, irq2in) != NULL; i++)
	{
		ClearTabSpace(Fpointer); //clears tabs and spaces
		irq2[i] = (int)strtoul(Fpointer, NULL, 10); //insret value to irq2 after coverting to int
	}
}
void ReadMemory()
{
	char Fpointer[MAX_LINE] = { 0 };
	int i = 0;
	while (1)
	{

		if (feof(memin)) // if reach EOF
		{
			break;
		}
		fgets(Fpointer, MAX_LINE, memin);//Read
		ClearTabSpace(Fpointer);//Cleat Tab and space
		mem[i] = (int)strtoul(Fpointer, NULL, 16);//conv str to int and save
		i++;
		for (int i = 0; i < 8; i++)//reset Fpointer
		{
			Fpointer[i] = 0;
		}
	}

}

void ClearTabSpace(char Hex[])
{
	int Num = 0;
	for (int i = 0; Hex[i] != '\0'; i++)// as long as the string doesnt end
	{
		Num = (int)(Hex[i]); //conv char to int
		if (Num == 32 || Num == 9 || Hex[i] == '\n') //if it is tab, space, or line drop
		{
			for (int j = i; Hex[j] != '\0'; j++)
			{
				Hex[j] = Hex[j + 1];//delete value and move all str one char left
			}
			i -= 1;	//for syncronising
		}
	}
}
int SignExtentionForImm(int Num)
{
	if (Num > 0x7FF)//check if negitive num
		return  -(0xFFF - Num + 1);
	return Num;

}
int ShiftRightLogic(int Num, int ShiftAmount)
{
	unsigned int newn = (unsigned int)Num;
	if (ShiftAmount == 0)
		return Num;
	return (int)(newn >> ShiftAmount);
}

void CloseAllFiles()
{
	if (memout != NULL)
		fclose(memout);
	if (leds != NULL)
		fclose(leds);
	if (trace != NULL)
		fclose(trace);
	if (regout != NULL)
		fclose(regout);
	if (hwregtrace != NULL)
		fclose(hwregtrace);
	if (cycles != NULL)
		fclose(cycles);
	if (display != NULL)
		fclose(display);
	if (diskout != NULL)
		fclose(diskout);
	if (diskin != NULL)
		fclose(diskin);
	if (memin != NULL)
		fclose(memin);
	if (irq2in != NULL)
		fclose(irq2in);
}

void AddToClock()
{
	if (IOR[8] == 0xFFFFFFFF) // if the clock reached his full capacitance
	{
		IOR[8] = 0;
	}
	else
		IOR[8] += 1; //add 1 to the clock
}
void AddToTimer()//add time to timer
{
	if (IOR[12] == IOR[13] && IOR[11] == 1)// checks if timer reached his max value
	{
		IOR[3] = 1; //turn on irq0
		IOR[12] = 0; //set timer to zero
	}
	else if (IOR[11] == 1)
	{
		IOR[12]++;
	}
}

void IN()
{
	if (R[rs] + R[rt] >= 0 && R[rs] + R[rt] <= 17 && rd != 0)//checks if register are valid
	{
		R[rd] = IOR[R[rs] + R[rt]];
	}
	PrintINHwregtrace(R[rs] + R[rt]);
}
void OUT()
{
	if (R[rs] + R[rt] >= 0 && R[rs] + R[rt] <= 17)//checks if register are valid (doesn't let to change zero)
	{
		switch (R[rs] + R[rt])//checks what type of out command we need to do
		{
		case 0: //if we are writing to irq0enabled
			IOR[R[rs] + R[rt]] = (unsigned int)R[rd] & 0x00000001;
			break;
		case 1: //if we are writing to irq1enabled
			IOR[R[rs] + R[rt]] = (unsigned int)R[rd] & 0x00000001;
			break;
		case 2: //if we are writing to irq2enabled
			IOR[R[rs] + R[rt]] = (unsigned int)R[rd] & 0x00000001;
			break;
		case 3://if we are writing to irq0status
			IOR[R[rs] + R[rt]] = (unsigned int)R[rd] & 0x00000001;
			break;
		case 4://if we are writing to irq1status
			IOR[R[rs] + R[rt]] = (unsigned int)R[rd] & 0x00000001;
			break;
		case 5://if we are writing to irq2status
			IOR[R[rs] + R[rt]] = (unsigned int)R[rd] & 0x00000001;
			break;
		case 6://if we are writing to irqhandler
			IOR[R[rs] + R[rt]] = (unsigned int)R[rd] & 0x00000fff;
			break;
		case 7://if we are writing to irqreturn
			IOR[R[rs] + R[rt]] = (unsigned int)R[rd] & 0x00000fff;
			break;
		case 8: //clk is read only!
			break;
		case 9://if we are writing to leds
			IOR[R[rs] + R[rt]] = (unsigned int)R[rd];
			PrintLeds();
			break;
		case 10: //if it is writing to display
			IOR[R[rs] + R[rt]] = (unsigned int)R[rd];
			PrintDisplay(IOR[R[rs] + R[rt]]);
			break;
		case 11://if we are writing to timerenable
			IOR[R[rs] + R[rt]] = (unsigned int)R[rd] & 0x00000001;
			break;
		case 14://if we are writing to diskcmd and read data from disk or write
			if (IOR[17] == 1)// if sidk is busy
				return;
			IOR[R[rs] + R[rt]] = (unsigned int)R[rd] & 0x00000003;
			if (IOR[R[rs] + R[rt]] == 1)//if we need to read
			{
				IOR[17] = 1; //disk busy
				for (int i = 0; i < NUM_WORDS_IN_SECTOR; i++)//insert info from disk to mem
				{
					mem[IOR[16] + i] = diskinmem[IOR[15]][i]; 
				}
			}
			if (IOR[R[rs] + R[rt]] == 2)//if we need to write
			{
				IOR[17] = 1;//disk busy
				for (int i = 0; i < NUM_WORDS_IN_SECTOR; i++)//insert info from mem to disk
				{
					diskinmem[IOR[15]][i] = mem[IOR[16] + i];
				}
			}
			break;
		case 15://if we are writing to disksector
			if (IOR[17] == 1)
				return;
			IOR[R[rs] + R[rt]] = ((unsigned int)R[rd] & 0x0000007f);
			break;
		case 16://if we are writing to diskbuffer
			if (IOR[17] == 1)
				return;
			IOR[R[rs] + R[rt]] = (unsigned int)R[rd] & 0x00000fff;
			break;
		case 17://if we are writing to diskstatus
			IOR[R[rs] + R[rt]] = (unsigned int)R[rd] & 0x00000001;
			break;
		default://if we are writing to register that we dont need a special case for
			IOR[R[rs] + R[rt]] = (unsigned int)R[rd];
			break;
		}

		PrintOUTHwregtrace(R[rs] + R[rt]);//print the changed register to hwregtrace.txt
	}
}

void Action()// does the command
{
	command = mem[PC]; //save currnt command
	opcode = command / 0x01000000;//save current opcode
	rd = (command / 0x00100000) % 16;// save current rd
	rs = (command / 0x00010000) % 16;// save current rs
	rt = (command / 0x00001000) % 16;// save current rt
	imm = (command) % 0x00001000;//save immediate
	R[1] = SignExtentionForImm(imm);//saves imm in R[1] with sign extention
	PrintToTrace(PC); //prints data of operation to trace
	if (rd >= 0 && rd <= 15 && rs >= 0 && rs <= 15 && rt >= 0 && rt <= 15)//checks if register are valid and not overflow
	{
		switch (opcode)//checks what command we need to do
		{
		case 0://ADD
			R[rd] = R[rs] + R[rt];
			R[0] = 0;//if someone tries to change register zero, the value will stay 0
			break;
		case 1://SUB
			R[rd] = R[rs] - R[rt];
			R[0] = 0;//if someone tries to change register zero, the value will stay 0
			break;
		case 2://AND
			R[rd] = R[rs] & R[rt];
			R[0] = 0;//if someone tries to change register zero, the value will stay 0
			break;
		case 3://OR
			R[rd] = R[rs] | R[rt];
			R[0] = 0;//if someone tries to change register zero, the value will stay 0
			break;
		case 4://SLL
			R[rd] = R[rs] << R[rt];
			R[0] = 0;//if someone tries to change register zero, the value will stay 0
			break;
		case 5://SRA
			R[rd] = R[rs] >> R[rt];
			R[0] = 0;//if someone tries to change register zero, the value will stay 0
			break;
		case 6://SRL
			R[rd] = ShiftRightLogic(R[rs], R[rt]);
			R[0] = 0;//if someone tries to change register zero, the value will stay 0
			break;
		case 7://BEQ
			if (R[rs] == R[rt])
			{
				PC = R[rd] & 0x00000FFF;
				jumpedNow = true;//says that we chnged the PC and dont need to PC++
			}
			break;
		case 8://BNE
			if (R[rs] != R[rt])
			{
				PC = R[rd] & 0x00000FFF;
				jumpedNow = true;//says that we chnged the PC and dont need to PC++
			}
			break;
		case 9://BLT
			if (R[rs] < R[rt])
			{
				PC = R[rd] & 0x00000FFF;
				jumpedNow = true;//says that we chnged the PC and dont need to PC++
			}
			break;
		case 10://BGT
			if (R[rs] > R[rt])
			{
				PC = R[rd] & 0x00000FFF;
				jumpedNow = true;//says that we chnged the PC and dont need to PC++
			}
			break;
		case 11://BLE
			if (R[rs] <= R[rt])
			{
				PC = R[rd] & 0x00000FFF;
				jumpedNow = true;//says that we chnged the PC and dont need to PC++
			}
			break;
		case 12://BGE
			if (R[rs] >= R[rt])
			{
				PC = R[rd] & 0x00000FFF;
				jumpedNow = true;//says that we chnged the PC and dont need to PC++
			}
			break;
		case 13://JAL
			R[15] = PC + 1;
			PC = R[rd] & 0x00000FFF;
			jumpedNow = true;//says that we chnged the PC and dont need to PC++
			break;
		case 14://LW
			if (R[rs] + R[rt] >= 0 && R[rs] + R[rt] < MAX_LEN_OF_FILE)
			{
				R[rd] = mem[R[rs] + R[rt]];
				R[0] = 0;//if someone tries to change register zero, the value will stay 0
			}
			break;
		case 15://SW
			if (R[rs] + R[rt] >= 0 && R[rs] + R[rt] < MAX_LEN_OF_FILE)
				mem[R[rs] + R[rt]] = R[rd];
			break;
		case 16://RETI
			PC = IOR[7];
			inIrq = false;
			jumpedNow = true;//says that we chnged the PC and dont need to PC++
			break;
		case 17://IN
			IN();
			break;
		case 18://OUT
			OUT();
			break;
		case 19://HALT
			finalHalt = 1;
			break;
		default:
			break;
		}
	}
}
void CheckEndOfWriteToDisk()//Checks If disk finished his write/read command
{
	if (CurrCycleOfStartDIsk == 1024)//checks if there have been 1024 Cycles since we started to work with disk
	{
		IOR[17] = 0; //disk status is free
		IOR[14] = 0; //disk cmd is 0
		IOR[4] = 1; //irq 1 status turn on
		CurrCycleOfStartDIsk = 0;
	}
	else if (IOR[17] == 1)// if we are busy but there have NOT been 1024 Cycles since we started to work with disk
		CurrCycleOfStartDIsk++; // add to cycle counter for disk
	else // if disk is free
		CurrCycleOfStartDIsk = 0;
}

void PrintDisplay(unsigned int Displaynum)//prints to Display 
{
	fprintf(display, "%ld %08X\n", TotalCycles, Displaynum);//print to display.txt
}
void PrintOUTHwregtrace(int numOfReg)
{
	switch (numOfReg) //checks which case we in and print right value
	{
	case 0:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "WRITE", "irq0enable", IOR[numOfReg]);
		break;
	case 1:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "WRITE", "irq1enable", IOR[numOfReg]);
		break;
	case 2:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "WRITE", "irq2enable", IOR[numOfReg]);
		break;
	case 3:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "WRITE", "irq0status", IOR[numOfReg]);
		break;
	case 4:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "WRITE", "irq1status", IOR[numOfReg]);
		break;
	case 5:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "WRITE", "irq2status", IOR[numOfReg]);
		break;
	case 6:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "WRITE", "irqhandler", IOR[numOfReg]);
		break;
	case 7:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "WRITE", "irqreturn", IOR[numOfReg]);
		break;
	case 8:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "WRITE", "clks", IOR[numOfReg]);
		break;
	case 9:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "WRITE", "leds", IOR[numOfReg]);
		break;
	case 10:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "WRITE", "display", IOR[numOfReg]);
		break;
	case 11:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "WRITE", "timerenable", IOR[numOfReg]);
		break;
	case 12:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "WRITE", "timercurrent", IOR[numOfReg]);
		break;
	case 13:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "WRITE", "timermax", IOR[numOfReg]);
		break;
	case 14:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "WRITE", "diskcmd", IOR[numOfReg]);
		break;
	case 15:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "WRITE", "disksector", IOR[numOfReg]);
		break;
	case 16:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "WRITE", "diskbuffer", IOR[numOfReg]);
		break;
	case 17:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "WRITE", "diskstatus", IOR[numOfReg]);
		break;
	}
}
void PrintINHwregtrace(int numOfReg)
{
	switch (numOfReg)
	{
	case 0:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "READ", "irq0enable", IOR[numOfReg]);
		break;
	case 1:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "READ", "irq1enable", IOR[numOfReg]);
		break;
	case 2:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "READ", "irq2enable", IOR[numOfReg]);
		break;
	case 3:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "READ", "irq0status", IOR[numOfReg]);
		break;
	case 4:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "READ", "irq1status", IOR[numOfReg]);
		break;
	case 5:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "READ", "irq2status", IOR[numOfReg]);
		break;
	case 6:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "READ", "irqhandler", IOR[numOfReg]);
		break;
	case 7:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "READ", "irqreturn", IOR[numOfReg]);
		break;
	case 8:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "READ", "clks", IOR[numOfReg]);
		break;
	case 9:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "READ", "leds", IOR[numOfReg]);
		break;
	case 10:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "READ", "display", IOR[numOfReg]);
		break;
	case 11:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "READ", "timerenable", IOR[numOfReg]);
		break;
	case 12:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "READ", "timercurrent", IOR[numOfReg]);
		break;
	case 13:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "READ", "timermax", IOR[numOfReg]);
		break;
	case 14:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "READ", "diskcmd", IOR[numOfReg]);
		break;
	case 15:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "READ", "disksector", IOR[numOfReg]);
		break;
	case 16:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "READ", "diskbuffer", IOR[numOfReg]);
		break;
	case 17:
		fprintf(hwregtrace, "%ld %s %s %08X\n", TotalCycles, "READ", "diskstatus", IOR[numOfReg]);
		break;
	}
}
void PrintLeds()
{
	fprintf(leds, "%ld %08X\n", TotalCycles, IOR[R[rs] + R[rt]]);
}
void PrintToMemout()//print mem to memout
{
	for (int i = 0; i < MAX_LEN_OF_FILE; i++)
	{
		fprintf(memout, "%08X\n", mem[i]);
	}
}
void PrintToCycles()//print TotalCycles to cycles
{
	fprintf(cycles, "%ld", TotalCycles);
}
void PrintToTrace(int PC)
{
	fprintf(trace, "%08X ", PC); //print pc to trace
	fprintf(trace, "%08X ", command); //print command to trace
	for (int i = 0; i <= 14; i++) //print all the registers to trace
	{
		fprintf(trace, "%08X ", R[i]);
	}
	fprintf(trace, "%08X\n", R[15]);
}
void PrintToDiskOut()
{
	for (int i = 0; i < NUM_SECTORS; i++)
	{
		for (int j = 0; j < NUM_WORDS_IN_SECTOR; j++)
		{
			fprintf(diskout, "%08X\n", diskinmem[i][j]);
		}

	}
}
void PrintReg()
{
	for (int i = 2; i <= 15; i++)
		fprintf(regout, "%08X\n", R[i]);
}



