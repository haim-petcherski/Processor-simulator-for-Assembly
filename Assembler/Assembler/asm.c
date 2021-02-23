#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>


#define MAX_LINE_LEN			500
#define MAX_LABEL_LEN			50
#define MAX_OPLINES				4096
#define NUM_OF_OPCODES			20		 
#define NUM_OF_REGS				16

//enum to organize the input argv
typedef enum _arg {
	INPUT_FILE = 1,
	OUTPUT_FILE,
	ARG_COUNT
}Arg;

//all of the main functions return a status enum for debugging
typedef enum _status {
	INVALID_STATUS_CODE = -1,
	SUCCESS = 0,
	WRONG_ARGUMENT_COUNT,
	INPUT_FOPEN_FAIL,
	OUTPUT_FOPEN_FAIL,
	NO_ADDRESS_IN_WORD,
	NO_DATA_IN_WORD,
	INCORRECT_LABEL,
	NO_OPCODE,
	NO_RD,
	NO_RS,
	NO_RT,
	NO_IMMEDIATE,
	UNKNOWN_OPCODE,
	UNKNOWN_REGISTER,
	UNKNOWN_LABEL
}Status;

//enum to pass to the implemented find function
typedef enum _type {
	OPCODE,
	REGISTER,
	LABEL
}Type;

//type built for oplines
//contains all the necessary fields in int type 
//and an additional label field to store the label name to translate later to the labels pc in the immediate field
typedef struct _opline {
	int immediate;
	int rt;
	int rs;
	int rd;
	int opcode;
	char label[MAX_LABEL_LEN];
}Opline;

//type built for .word commands
//contains the necessary address and data fields in int type
//and an additional pc field to store the pc that the command appeared in the input
typedef struct _word {
	int address;
	unsigned long data;
	int pc;
}Word;

//type built for the labels
//contains the name of the label and the pc it appears in the input file
typedef struct _label {
	char name[MAX_LABEL_LEN];
	int pc;
}Label;


//global arrays containing the supported opcodes and registers
//organized so that the index of each opcode/register is its number
char *Opcodes[NUM_OF_OPCODES] = { "add", "sub", "and", "or", "sll", "sra", "srl", "beq", "bne", "blt", "bgt", "ble", "bge", "jal", "lw", "sw", "reti", "in", "out", "halt" };
char *Regisers[NUM_OF_REGS] = { "$zero", "$imm", "$v0", "$a0", "$a1", "$t0", "$t1", "$t2", "$t3", "$s0", "$s1", "$s2", "$gp", "$sp", "$fp", "$ra" };

//global arrays containing the labels, oplines and .word commands
Label labels[MAX_OPLINES];
Opline oplines[MAX_OPLINES];
Word words[MAX_OPLINES];

//global counters, pc for the opcodes, and word/label_count for the .words/labels appearing in the input file
int pc = 0;
int word_count = 0;
int label_count = 0;


//the functions prototypes
Status read_input(FILE *input_file);
Status write_output(FILE *output_file);
Status update_word(char *start);
Status update_opline(char *start);
int find(char *token, Type type);


//the main function
//opens and closes the input/output files
//calls the read_input and write_output functions
//exits safely if something went wrong
Status main(int argc, char *argv[]) {
	Status status = INVALID_STATUS_CODE;
	FILE *input_file = NULL, *output_file = NULL;
	char *input_filename, *output_filename;

	//sorting input arguments
	if (argc != ARG_COUNT) exit(WRONG_ARGUMENT_COUNT);
	input_filename = argv[INPUT_FILE];
	output_filename = argv[OUTPUT_FILE];

	//INPUT
	input_file = fopen(input_filename, "r");
	if (input_file == NULL) exit(INPUT_FOPEN_FAIL);

	//first reading the input file and sorting the code
	//each label, .word and opline is saved in its global array
	status = read_input(input_file); 
	if (status) goto EXIT;
	fclose(input_file);


	//OUTPUT
	output_file = fopen(output_filename, "w");
	if (output_file == NULL) exit(OUTPUT_FOPEN_FAIL);

	//writing to the output file in the correct order in the needed format
	//translating the label names of the oplines to the pc it appears at
	status = write_output(output_file);
	if (status) goto EXIT;
	fclose(output_file);


	return status;

	//emergancy exit if something goes wrong
	//closes the unclosed files
EXIT:
	if (input_file != NULL) fclose(input_file);
	if (output_file != NULL) fclose(output_file);
	exit(status);
}
	


//a forever while loop that goes over each line of the input file at a time
//ignoring comments spaces and tabs
//saves label to global labels array
//calls the functions that sort the oplines and .word commands
Status read_input(FILE *input_file) {
	Status status = INVALID_STATUS_CODE;
	char line[MAX_LINE_LEN];
	char *label_end = NULL;
	char *comment = NULL;
	char *token = NULL;
	char *cur = NULL;
	
	//main forever loop
	while (1) {
		//reading each line and returns if reached EOF
		cur = fgets(line, MAX_LINE_LEN, input_file);	//cur holds a pointer to the start of the line or NULL if EOF reached
		if (cur == NULL) return SUCCESS;

		//getting a pointer to the first occurrence of '#' if exists and replacing it with NULL termination to ignore
		comment = strchr(line, '#');
		if (comment != NULL) *comment = '\0';

		//labels handling
		//getting a pointer to the first occurrence of ':' if a label exists
		//and saving it the global labels array
		label_end = strchr(line, ':');
		if (label_end != NULL) {
			token = strtok(line, " \t:");				//token holds the label name, ignoring spaces and tabs
			strcpy(labels[label_count].name, token);	//saves the label name
			labels[label_count].pc = pc;				//saves the pc that the label appears at
			label_count++;								//update label counter
			cur = label_end + 1;						//update cur to hold a pointer to what follows the label in the current line
		}
		
		//.word handling
		//looks for a .word command
		//if found updates the words global array
		token = strstr(cur, ".word");					//token points to the start of .word if found or NULL if not
		if (token != NULL) { 
			status = update_word(token);				//calls the update_word function that updates the data and address fields
			if (status) return status;
			words[word_count].pc = pc;					//saves the pc that the .word appears at
			word_count++;								//update .word counter
		}
		//opline handling
		//if .word command not found looks for opline by calling the update_opline function
		else {
			status = update_opline(cur);
			if (status) return status;
		} 
	}
}


//constracting the output in the correct order and writing each line in the needed format
//translating the label names of the oplines to the pc it appears at
//writing to the output file
Status write_output(FILE *output_file) {
	Status status = INVALID_STATUS_CODE;
	char output[MAX_OPLINES][10];						//each line contains 8 hexa characters, a newline charachter and a NULL termination
	int opline_c = 0;
	int word_c = 0;
	int padding_line_c = 0;
	int last_line = 0;
	char *line_ptr = NULL;


	//for each pc, first looking for .word commands and executing them in the order they appear if there are any
	//padding lines with zeros if needed to reach a higher address
	//then writing the oplines, translating the label names to their pc values
	//after finishing ordering all the output writing it to the output file
	for (opline_c = 0; opline_c <= pc; opline_c++) {
		
		//handling multiple words in the same pc
		//go over all the words that were not executed already and appear in the current pc and executes them
		//padding with zeros if needed
		while (word_c < word_count) {
			if (words[word_c].pc == opline_c) {

				//while didn't reach the needed address
				while (words[word_c].address > (opline_c + padding_line_c)) {								
					if ((opline_c + padding_line_c) > last_line)					//check if already written to that line
						sprintf(output[opline_c + padding_line_c], "%08X\n", 0);	//if didn't, pad with zeros
					padding_line_c++;
				}
				padding_line_c = 0;

				//write the data to the address
				sprintf(output[words[word_c].address], "%08X\n", words[word_c].data);

				//update the last line written (last in the sense that it appears last in the output file)
				if (last_line < words[word_c].address) last_line = words[word_c].address;
				word_c++;
			}

			//no more words in that pc
			else break; 
		}

		//write the opline until reaching the last (pc is always ahead by 1 because updated after the opline)
		if (opline_c < pc) {

			//if there's a label, update the immediate with the labels pc
			if (isalpha(oplines[opline_c].label[0])) {
				oplines[opline_c].immediate = find(oplines[opline_c].label, LABEL);		//using the find function to translate the label name to its pc value and saves it in the immediate field
				if (oplines[opline_c].immediate == -1) return UNKNOWN_LABEL;
			}

			//write the opline in the needed format
			sprintf(output[opline_c], "%02X%01X%01X%01X%03X\n", oplines[opline_c].opcode, oplines[opline_c].rd, oplines[opline_c].rs, oplines[opline_c].rt, oplines[opline_c].immediate);
			
			//update the last line written (last in the sense that it appears last in the output file)
			if (last_line < opline_c) last_line = opline_c;
		}
	}

	//write to the output file
	for (int i = 0; i <= last_line; i++) {
		fputs(output[i], output_file);
	}

	return SUCCESS;
}


//updates the address and data of the .word command
//gets as an argument the start of the .word comment
//saves the address and data to the global words array as int values
Status update_word(char *start) {
	Status status = INVALID_STATUS_CODE;
	char *token = NULL;

	//address handling
	token = strtok(start + 5, " \t\n");									//token holds the address in the string format ,ignoring the ".word", spaces and tabs
	if (token == NULL) return NO_ADDRESS_IN_WORD;
	
	words[word_count].address = strtol(token, NULL, 0) & 0xfff;			//converting the address to its integer value and using masking to limit maximum value 
																		//updating the address field in the global words array

	//data handling
	token = strtok(NULL, " \t\n");										//token holds the address in the string format ,ignoring spaces and tabs
	if (token == NULL) return NO_DATA_IN_WORD;
	
	words[word_count].data = strtoul(token, NULL, 0);					//converting the data to its integer value and updating the data field in the global words array

	return SUCCESS;
}


//updates the oplines global array
//gets as an argument the start of the line, ignoring the label if exists
//saves the opcodes and registers as their number value
//saves the label names as a string
//updates the pc counter
Status update_opline(char *start) {
	Status status = INVALID_STATUS_CODE;
	char *token = NULL;

	//opcode handling
	token = strtok(start, " \t\n");										//token holds the opcode if found or NULL if not, ignoring spaces, tabs and newline character
	if (token == NULL) return SUCCESS;									//no opcode means an empty line, so returns and continues to the next line

	oplines[pc].opcode = find(token, OPCODE);							//using the find function to translate the opcode to its number value and saves it in the opcode field
	if (oplines[pc].opcode == NUM_OF_OPCODES) return UNKNOWN_OPCODE;


	//rd handling
	token = strtok(NULL, " ,\t\n");										//token holds the rd if found, ignoring spaces, tabs and newline character
	if (token == NULL) return NO_RD;
	
	oplines[pc].rd = find(token, REGISTER);								//using the find function to translate the register to its number value and saves it in the rd field
	if (oplines[pc].rd == NUM_OF_REGS) return UNKNOWN_REGISTER;
	

	//rs handling
	token = strtok(NULL, " ,\t\n");										//token holds the rs if found, ignoring spaces, tabs and newline character
	if (token == NULL) return NO_RS;

	oplines[pc].rs = find(token, REGISTER);								//using the find function to translate the register to its number value and saves it in the rs field
	if (oplines[pc].rs == NUM_OF_REGS) return UNKNOWN_REGISTER;


	//rt handling
	token = strtok(NULL, " ,\t\n");										//token holds the rt if found, ignoring spaces, tabs and newline character
	if (token == NULL) return NO_RT;

	oplines[pc].rt = find(token, REGISTER);								//using the find function to translate the register to its number value and saves it in the rt field
	if (oplines[pc].rt == NUM_OF_REGS) return UNKNOWN_REGISTER;

	//immediate handling
	token = strtok(NULL, " ,\t\n");										//token holds the immediate if found, ignoring spaces, tabs and newline character
	if (token == NULL) return NO_IMMEDIATE;
	
	//if immediate is a label (if starts with an alphabetical letter)
	if (isalpha(*token)) { 
		strcpy(oplines[pc].label, token);								//saving the label name as a string to the label field
	}
	//otherwise the immediate is a number
	else { 
		oplines[pc].immediate = strtol(token, NULL, 0) & 0xfff;	//converting the immediate to its integer value, using masking to limit it to maximum value
	}

	//updating pc count
	pc++;

	return SUCCESS;
}


//an auxiliary function
//gets as an argument a name string and a type enum
//for register and opcode types it translates their name to their number value using the global registers/opcodes arrays
//for label type it translates the label name to the pc it appears at using the global labels array
int find(char *token, Type type) {
	int i = 0;

	switch (type) {
	case REGISTER:
		for (i = 0; i < NUM_OF_REGS; i++) {
			if (strcmp(token, Regisers[i]) == 0) return i;
		}
		return NUM_OF_REGS;

	case OPCODE:
		for (i = 0; i < NUM_OF_OPCODES; i++) {
			if (strcmp(token, Opcodes[i]) == 0) return i;
		}
		return NUM_OF_OPCODES;

	case LABEL:
		for (i = 0; i < label_count; i++) {
			if (strcmp(token, labels[i].name) == 0) return labels[i].pc;
		}
		return -1;
	}
}
