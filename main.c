/*
	Name 1: Bilal Quraishi
	Name 2: Darius Zinolabedini
	UTEID 1: buq57
	UTEID 2: djz333
*/

#include <stdio.h> /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h> /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */

#define MAX_LINE_LENGTH 255
#define MAX_LABEL_LEN 20 //Struct for the Symbol Table
#define MAX_SYMBOLS 255

enum {DONE, OK, EMPTY_LINE};
struct SymbolTableEntry
{
	int address;
	char label[MAX_LABEL_LEN + 1];
	int fillValue;
};
typedef struct SymbolTableEntry TableEntry;


int determineValidHex(char *startingAddress, int addressFlag);
int isOpcode(char *inLoc);
int readAndParse(FILE *pInfile, char *pLine, char **pLabel, char **pOpcode, char **pArg1, char **pArg2, char **pArg3, char **pArg4);
int firstPass(FILE *in, TableEntry symbolTable[], char *startingAddress);
void secondPass(FILE *in, FILE *out, TableEntry symbolTable[], char *startingAddress, int tableLoc);
int isDuplicate(char *labelo, TableEntry symbolTable[], int tableLoc, int tracker);
int isValidLabel(char *lLabel);
int isAlphanumeric(char *label);
int toNum(char * pStr);
int onlySpaces(char *spaces);
void seperateHex(char* bin, FILE *out);
void outputHex(char* digit, FILE *out);
void appendRegister(char *bin, char *lLabel);
void appendImmediate(char *bin, char *lLabel);
void toBinary(int num, int digits, char *result);
void twosComplement(char *in, int digits);
int isValidRegister(char *lLabel);
int isValidImmediate(char *lLabel);
void printSymbolTable(TableEntry symbolTable[], int tableLoc);
void toHexNoOutput(char *input, char *output);
void toHexNoOutputHelper(char *digit, char *out, int index);
void appendAmount(char *bin, char *lLabel);
void appendOffset(char *bin, char *lLabel);
void appendTrapVector(char *bin, char *lLabel);
int isValidAmount(char *lLabel);
int isValidOffset(char *lLabel);
int isValidTrapVector(char *lLabel);
int isInSymbolTable(char *lLabel, TableEntry symbolTable[], int tableLoc, int address);
int isInSymbolTable2(char *lLabel, TableEntry symbolTable[], int tableLoc, int address);
int isInSymbolTable3(char *lLabel, TableEntry symbolTable[], int tableLoc, int address);



int main(int argc, char* argv[])
{
	TableEntry symbolTable[MAX_SYMBOLS] = {0};
	FILE *infile;
	FILE *outfile;
	char pLine[MAX_LINE_LENGTH + 1] = {0}; //Initialize pointers to be passed
	char *pLabel, *pOpcode, *pArg1, *pArg2, *pArg3, *pArg4;
	char startingAddress[6] = {0};
	int addressFlag = 1;
	int tableLoc;

    infile = fopen(argv[1], "r");
    outfile = fopen(argv[2], "w");



    if (!infile)  //Open input file
    {
      printf("Error: Cannot open file %s.\n", argv[1]);
      exit(4);
    }
    if (!outfile) //Open output file
    {
      printf("Error: Cannot open file %s.\n", argv[2]);
      exit(4);
    }


    // ORIG checks//
    int returnValue = readAndParse(infile, pLine, &pLabel, &pOpcode, &pArg1, &pArg2, &pArg3, &pArg4); //Check for .ORIG
    while((strlen(pLabel) >= 0))
    {
    	if (strcmp(pOpcode, ".orig") == 0) break;
    	if (strlen(pLabel) > 0) exit(4);
    	returnValue = readAndParse(infile, pLine, &pLabel, &pOpcode, &pArg1, &pArg2, &pArg3, &pArg4); //next line
    }
    if (strcmp(pOpcode, ".orig") != 0) exit(4); //No .ORIG
    if (strlen(pArg1) != 5) exit(3); //Adress is incorrect size
    if ((strlen(pArg2) > 0) && (pArg2[0] != ';')) exit(4);
    if (pArg1[0] == '#') //If address is decimal
    {
    	char decimalAddress[17] = {0};
    	char result[6] = {0};
    	char digits[5] = {0};
    	toBinary(toNum(pArg1), 16, decimalAddress);
    	toHexNoOutput(decimalAddress, digits);
    	result[0] = 'x';
    	strcat(result, digits);
    	strncpy(startingAddress, result, sizeof(startingAddress));
    }
    else
		{
			strncpy(startingAddress, pArg1, sizeof(startingAddress));
			startingAddress[5] = '\0';
		}
		//Find starting address;
    if (determineValidHex(startingAddress, addressFlag) == 0) exit(3); //Incorrect address formatting

    /*First Pass*/
    tableLoc = firstPass(infile, symbolTable, startingAddress);
    rewind(infile);
    //printSymbolTable(symbolTable, tableLoc);
    fputs (startingAddress, outfile);
    fputs ("\n", outfile);
    secondPass(infile, outfile, symbolTable, startingAddress, tableLoc);



    fclose(infile); //Close files
    fclose(outfile);
    exit(0);
}

//Converts to hex without outputting
void toHexNoOutput(char *bin, char *out)
{
	char temp[5];
	temp[0] = bin[0]; temp[1] = bin[1]; temp[2] = bin[2]; temp[3] = bin[3]; temp[4] = '\0';
	toHexNoOutputHelper(temp, out, 0);
	temp[0] = bin[4]; temp[1] = bin[5]; temp[2] = bin[6]; temp[3] = bin[7]; temp[4] = '\0';
	toHexNoOutputHelper(temp, out, 1);
	temp[0] = bin[8]; temp[1] = bin[9]; temp[2] = bin[10]; temp[3] = bin[11]; temp[4] = '\0';
	toHexNoOutputHelper(temp, out, 2);
	temp[0] = bin[12]; temp[1] = bin[13]; temp[2] = bin[14]; temp[3] = bin[15]; temp[4] = '\0';
	toHexNoOutputHelper(temp, out, 3);
	out[4] = '\0';
}

//Helper function
void toHexNoOutputHelper(char *digit, char *out, int index)
{
	if(strcmp(digit, "0000") == 0) out[index] = '0';
	else if(strcmp(digit, "0001") == 0) out[index] = '1';
	else if(strcmp(digit, "0010") == 0) out[index] = '2';
	else if(strcmp(digit, "0011") == 0) out[index] = '3';
	else if(strcmp(digit, "0100") == 0) out[index] = '4';
	else if(strcmp(digit, "0101") == 0) out[index] = '5';
	else if(strcmp(digit, "0110") == 0) out[index] = '6';
	else if(strcmp(digit, "0111") == 0) out[index] = '7';
	else if(strcmp(digit, "1000") == 0) out[index] = '8';
	else if(strcmp(digit, "1001") == 0) out[index] = '9';
	else if(strcmp(digit, "1010") == 0) out[index] = 'A';
	else if(strcmp(digit, "1011") == 0) out[index] = 'B';
	else if(strcmp(digit, "1100") == 0) out[index] = 'C';
	else if(strcmp(digit, "1101") == 0) out[index] = 'D';
	else if(strcmp(digit, "1110") == 0) out[index] = 'E';
	else if(strcmp(digit, "1111") == 0) out[index] = 'F';
}

//Determines if an input is a valid hex value
int determineValidHex(char *startingAddress, int addressFlag)
{
	if (*startingAddress != 'x') return 0;
	for (int i = 1; i < 5; i++)
	{
		char temp = *(startingAddress + i);
		if ((temp >= 48 & temp <= 57) | (temp >= 97 & temp <= 102)) continue;
		return 0;
	}
	char t = *(startingAddress + 4);
	if (addressFlag == 0) return 1;
	if (t == '1' | t == '3' | t == '5' | t == '7' | t == '9' | t == 'B' | t == 'D' | t == 'F') return 0;
	return 1;
}

//Function to determine if the string is an opcode
int isOpcode(char *inLoc)
{
	/* Declaring all possibilities */
	char *list[31] = {"add", "and", "br", "brn", "brz", "brp", "brnz", "brnp", "brzp", "brnzp", "halt", "jmp", "jsr", "jsrr", "ldb", "ldw", "lea", "nop", "not", "ret", "lshf", "rshfl", "rshfa", "rti", "stb", "stw", "trap", "xor", ".orig", ".fill", ".end"};
	char *temp = inLoc;

	for(int i = 0; i < 31; i++) //Compare with all 28 Opcodes
	{
		int j = 0; //Pointer for 2D Array
		while (*temp != '\0') //While loop to check entirety of input
		{
			if (*temp != (list[i])[j]) break; //If the input is not a match for the opcode check the next opcode
			else
			{
				temp++; //Next letter of the input
				j++; //Next letter in this opcode
			}
		}
		if (*temp == '\0' & (list[i])[j] == '\0') return 1; //Input is an opcode
		temp = inLoc; //Reset input
	}
	return -1;
}

//Function to read and parse the line
int readAndParse(FILE *pInfile, char *pLine, char **pLabel, char **pOpcode, char **pArg1, char **pArg2, char **pArg3, char **pArg4)
{
	char * lRet, * lPtr;
	int i;
	if (!fgets(pLine, MAX_LINE_LENGTH, pInfile)) return(DONE);
	for (i = 0; i < strlen(pLine); i++) pLine[i] = tolower(pLine[i]);

	/* Convert entire line to lowercase */
	*pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = pLine + strlen(pLine);

	/* Ignore the comments */
	lPtr = pLine;

	while (*lPtr != ';' && *lPtr != '\0' && *lPtr != '\n') lPtr++;

	*lPtr = '\0';

	if (!(lPtr = strtok(pLine, "\t\n ,"))) return(EMPTY_LINE);

	/* Found a label */
	if (isOpcode(lPtr) == -1 && lPtr[0] != '.')
	{
		*pLabel = lPtr;
		if(!(lPtr = strtok( NULL, "\t\n ,"))) return(OK);
	}

	*pOpcode = lPtr;

	if (!(lPtr = strtok(NULL, "\t\n ,"))) return(OK);
	*pArg1 = lPtr;
	if (!(lPtr = strtok(NULL, "\t\n ,"))) return(OK);

	*pArg2 = lPtr;

	if (!(lPtr = strtok(NULL, "\t\n ,"))) return(OK);

	*pArg3 = lPtr;

	if (!(lPtr = strtok(NULL, "\t\n ,"))) return(OK);

	*pArg4 = lPtr;

	return(OK);
}

//Performs first pass opeartions
int firstPass(FILE *in, TableEntry symbolTable[], char *startingAddress)
{
   char lLine[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1, *lArg2, *lArg3, *lArg4;
   int lRet;
   int programCounter = 0;
   int tableLoc = 0;
   int starting = toNum(startingAddress);
   int notAddress = 0; //used for address flag
   int endLocated = 0; //Check if .END statement is present
   do
   {
        lRet = readAndParse(in, lLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4);
        if (lRet != DONE && lRet != EMPTY_LINE)
        {
        	if (lLabel[0] == ';') continue; //Check for comments
					if (strcmp(lOpcode, ".orig") == 0) //Check for erroneously placed .orig statement
					{
						exit(4);
					}
        	if (strcmp(lOpcode, ".end") == 0) //Check for .end statement
        	{
        		if(onlySpaces(lLabel) == 0 || onlySpaces(lArg1) == 0) exit(4);
        		endLocated = 1;
        		break;
        	}
        	if (isOpcode(lArg1) == 1) exit(4);
        	if (onlySpaces(lLabel) == 0) //Check if there is a label
        	{
        		if (isValidLabel(lLabel) == 0)
        		{
        			if(strlen(lOpcode) > 0) exit(2);
        			exit(4);
        		}
        		if (isDuplicate(lLabel, symbolTable, tableLoc, 0) == 0) //Check if the label has already been used
        		{
        			symbolTable[tableLoc].address = starting + programCounter + 2; //add the label
        			strncpy(symbolTable[tableLoc].label, lLabel, MAX_LABEL_LEN); //add the address
        			if (strcmp(lOpcode, ".fill") == 0) //Check if there is a .fill statement
        			{
								if (isValidLabel(lArg1)) {}
        				else if(lArg1[0] == '#');
        				else if(determineValidHex(lArg1, notAddress) == 0) exit(4); //Check if the hex value is valid (not address allows it to be odd)
        				if(onlySpaces(lArg2) == 0) exit(4); //Cant have a second argument
        				symbolTable[tableLoc].fillValue = toNum(lArg1); //Store value in symbol table
        			}
        		}
        		else exit(4);
        		tableLoc += 1;
        	}
        	programCounter += 2;
        }
   }
   while (lRet != DONE);
   if(endLocated == 0) exit(4); //No .end statement
   return tableLoc;
}

//Second pass of the assembler
void secondPass(FILE *in, FILE *out, TableEntry symbolTable[], char *startingAddress, int tableLoc)
{
	char lLine[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1, *lArg2, *lArg3, *lArg4;
    int lRet;
    int endLocated = 0;
    char bin[17] = {0};
    int programCounter = 0;
    int starting = toNum(startingAddress);
    int returnValue2;
    lRet = readAndParse( in, lLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4 );
    if (strcmp(lOpcode, ".orig") != 0) //Parse till .orig
    {
    	while((strlen(lLabel) >= 0))
   		{
    		if (strcmp(lOpcode, ".orig") == 0) break;
    		returnValue2 = readAndParse( in, lLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4 ); //next line
    	}
    }
    programCounter += 2;
    do
    {
        lRet = readAndParse( in, lLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4 );
        if( lRet != DONE && lRet != EMPTY_LINE )
        {
        	if (isOpcode(lOpcode) == -1) exit(2); //Check for valid opcode
        	if (lLabel[0] == ';') continue; //Check for comments
        	if (strcmp(lOpcode, ".end") == 0) //Check for .end statement
   			{
        		if(onlySpaces(lLabel) == 0 || onlySpaces(lArg1) == 0) exit(4);
        		endLocated = 1;
        		break;
    		}

    		if (strcmp(lOpcode, ".fill") == 0) //Check for .fil statement
    		{
					if (isValidLabel(lArg1))
					{
						char offsetChar[17] = {0};
						toBinary(isInSymbolTable3(lArg1, symbolTable, tableLoc, starting+programCounter+2), 16, offsetChar);
						strcat(bin, offsetChar);
						seperateHex(bin, out);
    				memset(bin, 0, 17); //Clear array
        			programCounter += 2;
    				continue;
					}
    			else if (strlen(lArg1) > 5) exit(4);
    			else if (lArg1[0] == '#')
    			{
    				toBinary(toNum(lArg1), 16, bin);
    				seperateHex(bin, out);
    				memset(bin, 0, 17); //Clear array
        			programCounter += 2;
    				continue;
    			}
    			else if (determineValidHex((lArg1), 0) == 1)
    			{
    				toBinary(toNum(lArg1), 16, bin);
    				seperateHex(bin, out);
    				memset(bin, 0, 17); //Clear array
        			programCounter += 2;
    				continue;
    			}
    			else exit(4);
    			if ((strlen(lArg2) > 0) && (lArg2[0] != ';')) exit(4);
    		}

        	if (strcmp(lOpcode, "add") == 0) //Check for add instruction
        	{
        		strncpy(bin, "0001", 4); //opcode
        		if (isValidRegister(lArg1) == 1) //dr
        		{
        			appendRegister(bin, lArg1);
        		}
        		else exit(4);
        		if (isValidRegister(lArg2) == 1) //sr1
        		{
        			appendRegister(bin, lArg2);
        		}
        		else exit(4);
        		if (isValidRegister(lArg3) == 1) //sr2
        		{
        			strcat(bin, "000");
        			appendRegister(bin, lArg3);
        		}
        		else if (isValidImmediate(lArg3) == 1) //imm5
        		{
        			strcat(bin, "1");
        			appendImmediate(bin, lArg3);
        		}
        		else exit(4);
        		if ((strlen(lArg4) > 0) && (lArg4[0] != ';')) exit(4);
        		seperateHex(bin, out);
        	}

        	if (strcmp(lOpcode, "and") == 0) //Check for and instruction
        	{
        		strncpy(bin, "0101", 4); //opcode
        		if (isValidRegister(lArg1) == 1) //dr
        		{
        			appendRegister(bin, lArg1);
        		}
        		else exit(4);
        		if (isValidRegister(lArg2) == 1) //sr1
        		{
        			appendRegister(bin, lArg2);
        		}
        		else exit(4);
        		if (isValidRegister(lArg3) == 1) //sr2
        		{
        			strcat(bin, "000");
        			appendRegister(bin, lArg3);
        		}
        		else if (isValidImmediate(lArg3) == 1) //imm5
        		{
        			strcat(bin, "1");
        			appendImmediate(bin, lArg3);
        		}
        		else exit(4);
        		if ((strlen(lArg4) > 0) && (lArg4[0] != ';')) exit(4);
        		seperateHex(bin, out);
        	}

        	if (strcmp(lOpcode, "jmp") == 0) //Check for jmp instruction
        	{
        		strncpy(bin, "1100", 4); //opcode
        		strcat(bin,"000");
        		if (isValidRegister(lArg1) == 1) //dr
        		{
        			appendRegister(bin, lArg1);
        		}
        		else exit(4);
        		strcat(bin, "000000");
        		if ((strlen(lArg2) > 0) && (lArg2[0] != ';')) exit(4);
        		seperateHex(bin, out);
        	}

        	if (strcmp(lOpcode, "nop") == 0) //Check for nop instruction
        	{
        		if ((strlen(lArg1) > 0) && (lArg1[0] != ';')) exit(4);
        		strcat(bin, "0000000000000000"); //whole instruction
        		seperateHex(bin, out);
        	}

        	if (strcmp(lOpcode, "xor") == 0) //Check for xor instruction
        	{
        		strncpy(bin, "1001", 4); //opcode
        		if (isValidRegister(lArg1) == 1) //dr
        		{
        			appendRegister(bin, lArg1);
        		}
        		else exit(4);
        		if (isValidRegister(lArg2) == 1) //sr1
        		{
        			appendRegister(bin, lArg2);
        		}
        		else exit(4);
        		if (isValidRegister(lArg3) == 1) //sr2
        		{
        			strcat(bin, "000");
        			appendRegister(bin, lArg3);
        		}
        		else if (isValidImmediate(lArg3) == 1) //imm5
        		{
        			strcat(bin, "1");
        			appendImmediate(bin, lArg3);
        		}
        		else exit(4);
        		if ((strlen(lArg4) > 0) && (lArg4[0] != ';')) exit(4);
        		seperateHex(bin, out);
        	}

        	if (strcmp(lOpcode, "jsrr") == 0) //Check for jsrr instruction
        	{
        		strncpy(bin, "0100", 4); //opcode
        		strcat(bin, "000");
        		if (isValidRegister(lArg1) == 1) //baser
        		{
        			appendRegister(bin, lArg1);
        		}
        		else exit(4);
        		if ((strlen(lArg2) > 0) && (lArg2[0] != ';')) exit(4);
        		strcat(bin, "000000");
        		seperateHex(bin, out);
        	}

        	if (strcmp(lOpcode, "not") == 0) //Check for not instruction
        	{
        		strncpy(bin, "1001", 4); //opcode
        		if (isValidRegister(lArg1) == 1) //dr
        		{
        			appendRegister(bin, lArg1);
        		}
        		else exit(4);
        		if (isValidRegister(lArg2) == 1) //sr1
        		{
        			appendRegister(bin, lArg2);
        		}
        		else exit(4);
        		if ((strlen(lArg3) > 0) && (lArg3[0] != ';')) exit(4);
        		strcat(bin, "111111");
        		seperateHex(bin, out);
        	}

        	if (strcmp(lOpcode, "ret") == 0) //Check for ret instruction
        	{
        		strcat(bin, "1100000111000000");
        		if ((strlen(lArg1) > 0) && (lArg1[0] != ';')) exit(4);
        		seperateHex(bin, out);
        	}

        	if (strcmp(lOpcode, "rti") == 0) //Check for rti instruction
        	{
        		strcat(bin, "1000000000000000");
        		if ((strlen(lArg1) > 0) && (lArg1[0] != ';')) exit(4);
        		seperateHex(bin, out);
        	}

        	if (strcmp(lOpcode, "lshf") == 0) //Check for lshf instruction
        	{
        		strcat(bin, "1101");
        		if (isValidRegister(lArg1) == 1) //dr
        		{
        			appendRegister(bin, lArg1);
        		}
        		else exit(4);
        		if (isValidRegister(lArg2) == 1) //sr1
        		{
        			appendRegister(bin, lArg2);
        		}
        		else exit(4);
        		strcat(bin, "00");
        		if (isValidAmount(lArg3) == 1) //amount4
        		{
        			appendAmount(bin, lArg3);
        		}
        		else exit(4);
        		if ((strlen(lArg4) > 0) && (lArg4[0] != ';')) exit(4);
        		seperateHex(bin, out);
        	}


        	if (strcmp(lOpcode, "rshfl") == 0) //Check for rshfl instruction
        	{
        		strcat(bin, "1101");
        		if (isValidRegister(lArg1) == 1) //dr
        		{
        			appendRegister(bin, lArg1);
        		}
        		else exit(4);
        		if (isValidRegister(lArg2) == 1) //sr1
        		{
        			appendRegister(bin, lArg2);
        		}
        		else exit(4);
        		strcat(bin, "01");
        		if (isValidAmount(lArg3) == 1) //amount4
        		{
        			appendAmount(bin, lArg3);
        		}
        		else exit(4);
        		if ((strlen(lArg4) > 0) && (lArg4[0] != ';')) exit(4);
        		seperateHex(bin, out);
        	}

        	if (strcmp(lOpcode, "rshfa") == 0) //Check for rshfa instruction
        	{
        		strcat(bin, "1101");
        		if (isValidRegister(lArg1) == 1) //dr
        		{
        			appendRegister(bin, lArg1);
        		}
        		else exit(4);
        		if (isValidRegister(lArg2) == 1) //sr1
        		{
        			appendRegister(bin, lArg2);
        		}
        		else exit(4);
        		strcat(bin, "11");
        		if (isValidAmount(lArg3) == 1) //amount4
        		{
        			appendAmount(bin, lArg3);
        		}
        		else exit(4);
        		if ((strlen(lArg4) > 0) && (lArg4[0] != ';')) exit(4);
        		seperateHex(bin, out);
        	}

        	if (strcmp(lOpcode, "ldb") == 0) //check for ldb
        	{
        		strcat(bin, "0010");
        		if (isValidRegister(lArg1) == 1) //dr
        		{
        			appendRegister(bin, lArg1);
        		}
        		else exit(4);
        		if (isValidRegister(lArg2) == 1) //baser
        		{
        			appendRegister(bin, lArg2);
        		}
        		else exit(4);
        		if (isValidOffset(lArg3) == 1) //offset6
        		{
        			appendOffset(bin, lArg3);
        		}
        		else exit(4);
        		if ((strlen(lArg4) > 0) && (lArg4[0] != ';')) exit(4);
        		seperateHex(bin, out);
       		}

       		if (strcmp(lOpcode, "ldw") == 0) //check for ldw
        	{
        		strcat(bin, "0110");
        		if (isValidRegister(lArg1) == 1) //dr
        		{
        			appendRegister(bin, lArg1);
        		}
        		else exit(4);
        		if (isValidRegister(lArg2) == 1) //baser
        		{
        			appendRegister(bin, lArg2);
        		}
        		else exit(4);
        		if (isValidOffset(lArg3) == 1) //offset6
        		{
        			appendOffset(bin, lArg3);
        		}
        		else exit(4);
        		if ((strlen(lArg4) > 0) && (lArg4[0] != ';')) exit(4);
        		seperateHex(bin, out);
       		}

       		if (strcmp(lOpcode, "stb") == 0) //check for stb
        	{
        		strcat(bin, "0011");
        		if (isValidRegister(lArg1) == 1) //sr
        		{
        			appendRegister(bin, lArg1);
        		}
        		else exit(4);
        		if (isValidRegister(lArg2) == 1) //baser
        		{
        			appendRegister(bin, lArg2);
        		}
        		else exit(4);
        		if (isValidOffset(lArg3) == 1) //offset6
        		{
        			appendOffset(bin, lArg3);
        		}
        		else exit(4);
        		if ((strlen(lArg4) > 0) && (lArg4[0] != ';')) exit(4);
        		seperateHex(bin, out);
       		}

       		if (strcmp(lOpcode, "stw") == 0) //check for stb
        	{
        		strcat(bin, "0111");
        		if (isValidRegister(lArg1) == 1) //sr
        		{
        			appendRegister(bin, lArg1);
        		}
        		else exit(4);
        		if (isValidRegister(lArg2) == 1) //baser
        		{
        			appendRegister(bin, lArg2);
        		}
        		else exit(4);
        		if (isValidOffset(lArg3) == 1) //offset6
        		{
        			appendOffset(bin, lArg3);
        		}
        		else exit(4);
        		if ((strlen(lArg4) > 0) && (lArg4[0] != ';')) exit(4);
        		seperateHex(bin, out);
       		}

       		if (strcmp(lOpcode, "halt") == 0) //check for halt
       		{
       			strcat(bin, "1111000000100101");
       			if ((strlen(lArg1) > 0) && (lArg1[0] != ';')) exit(4);
       			seperateHex(bin, out);
       		}

       		if (strcmp(lOpcode, "trap") == 0) //check for trapvector
       		{
       			strcat(bin, "11110000");
   				if (lArg1[0] != 'x') exit(4); //Must start with x
   				if (strlen(lArg1) > 3) exit(4); //Must be less than three chars
   				char temp[6];
   				temp[0] = 'x'; temp[1] = '0'; temp[2] = '0'; temp[3] = lArg1[1]; temp[4] = lArg1[2]; temp[5] = '\0';
   				if (determineValidHex(temp, 0) == 0) exit(4);
       			if (isValidTrapVector(lArg1) == 1)
       			{
       				appendTrapVector(bin, lArg1);
       			}
       			else exit(4);
       			if ((strlen(lArg2) > 0) && (lArg2[0] != ';')) exit(4);
       			seperateHex(bin, out);
       		}

       		if (strcmp(lOpcode, "lea") == 0) //check for lea
       		{
       			strcat(bin, "1110");
        		if (isValidRegister(lArg1) == 1) //dr
        		{
        			appendRegister(bin, lArg1);
        		}
        		if (isInSymbolTable(lArg2, symbolTable, tableLoc, starting+programCounter+2) != -999) //Offset9
        		{
        			char offsetChar[10] = {0};
        			toBinary(isInSymbolTable(lArg2, symbolTable, tableLoc, starting+programCounter+2), 9, offsetChar);
        			strcat(bin, offsetChar);
        		}
        		else exit(1);
        		if ((strlen(lArg3) > 0) && (lArg3[0] != ';')) exit(4);
       			seperateHex(bin, out);
       		}

       		if (strcmp(lOpcode, "br") == 0 | strcmp(lOpcode, "brnzp") == 0) //check for brnzp
       		{
       			strcat(bin, "0000111");
       			if (isInSymbolTable(lArg1, symbolTable, tableLoc, starting+programCounter+2) != -999) //Offset9
        		{
        			char offsetChar[10] = {0};
        			toBinary(isInSymbolTable(lArg1, symbolTable, tableLoc, starting+programCounter+2), 9, offsetChar);
        			strcat(bin, offsetChar);
        		}
        		else exit(1);
        		if ((strlen(lArg2) > 0) && (lArg2[0] != ';')) exit(4);
       			seperateHex(bin, out);
       		}

       		if (strcmp(lOpcode, "brn") == 0) //check for brn
       		{
       			strcat(bin, "0000100");
       			if (isInSymbolTable(lArg1, symbolTable, tableLoc, starting+programCounter+2) != -999) //Offset9
        		{
        			char offsetChar[10] = {0};
        			toBinary(isInSymbolTable(lArg1, symbolTable, tableLoc, starting+programCounter+2), 9, offsetChar);
        			strcat(bin, offsetChar);
        		}
        		else exit(1);
        		if ((strlen(lArg2) > 0) && (lArg2[0] != ';')) exit(4);
       			seperateHex(bin, out);
       		}

       		if (strcmp(lOpcode, "brnz") == 0) //check for brnz
       		{
       			strcat(bin, "0000110");
       			if (isInSymbolTable(lArg1, symbolTable, tableLoc, starting+programCounter+2) != -999) //Offset9
        		{
        			char offsetChar[10] = {0};
        			toBinary(isInSymbolTable(lArg1, symbolTable, tableLoc, starting+programCounter+2), 9, offsetChar);
        			strcat(bin, offsetChar);
        		}
        		else exit(1);
        		if ((strlen(lArg2) > 0) && (lArg2[0] != ';')) exit(4);
       			seperateHex(bin, out);
       		}

       		if (strcmp(lOpcode, "brnp") == 0) //check for brnp
       		{
       			strcat(bin, "0000101");
       			if (isInSymbolTable(lArg1, symbolTable, tableLoc, starting+programCounter+2) != -999) //Offset9
        		{
        			char offsetChar[10] = {0};
        			toBinary(isInSymbolTable(lArg1, symbolTable, tableLoc, starting+programCounter+2), 9, offsetChar);
        			strcat(bin, offsetChar);
        		}
        		else exit(1);
        		if ((strlen(lArg2) > 0) && (lArg2[0] != ';')) exit(4);
       			seperateHex(bin, out);
       		}

       		if (strcmp(lOpcode, "brz") == 0) //check for brz
       		{
       			strcat(bin, "0000010");
       			if (isInSymbolTable(lArg1, symbolTable, tableLoc, starting+programCounter+2) != -999) //Offset9
        		{
        			char offsetChar[10] = {0};
        			toBinary(isInSymbolTable(lArg1, symbolTable, tableLoc, starting+programCounter+2), 9, offsetChar);
        			strcat(bin, offsetChar);
        		}
        		else exit(1);
        		if ((strlen(lArg2) > 0) && (lArg2[0] != ';')) exit(4);
       			seperateHex(bin, out);
       		}

    		if (strcmp(lOpcode, "brp") == 0) //check for brp
       		{
       			strcat(bin, "0000001");
       			if (isInSymbolTable(lArg1, symbolTable, tableLoc, starting+programCounter+2) != -999) //Offset9
        		{
        			char offsetChar[10] = {0};
        			toBinary(isInSymbolTable(lArg1, symbolTable, tableLoc, starting+programCounter+2), 9, offsetChar);
        			strcat(bin, offsetChar);
        		}
        		else exit(1);
        		if ((strlen(lArg2) > 0) && (lArg2[0] != ';')) exit(4);
       			seperateHex(bin, out);
       		}


       		if (strcmp(lOpcode, "brzp") == 0) //check for brzp
       		{
       			strcat(bin, "0000011");
       			if (isInSymbolTable(lArg1, symbolTable, tableLoc, starting+programCounter+2) != -999) //Offset9
        		{
        			char offsetChar[10] = {0};
        			toBinary(isInSymbolTable(lArg1, symbolTable, tableLoc, starting+programCounter+2), 9, offsetChar);
        			strcat(bin, offsetChar);
        		}
        		else exit(1);
        		if ((strlen(lArg2) > 0) && (lArg2[0] != ';')) exit(4);
       			seperateHex(bin, out);
       		}

       		if (strcmp(lOpcode, "jsr") == 0) //check for jsr
       		{
       			strcat(bin, "01001");
       			if (isInSymbolTable2(lArg1, symbolTable, tableLoc, starting+programCounter+2) != -999) //Offset11
        		{
        			char offsetChar[12] = {0};
        			toBinary(isInSymbolTable2(lArg1, symbolTable, tableLoc, starting+programCounter+2), 11, offsetChar);
        			strcat(bin, offsetChar);
        		}
        		else exit(1);
        		if ((strlen(lArg2) > 0) && (lArg2[0] != ';')) exit(4);
       			seperateHex(bin, out);
       		}
        	memset(bin, 0, 17); //Clear array
        	programCounter += 2;
        }
    }
    while( lRet != DONE );
}

//Checks if something is in the symbol table
int isInSymbolTable(char *lLabel, TableEntry symbolTable[], int tableLoc, int address)
{
	for (int i = 0; i < tableLoc; i++)
	{
		if (strcmp(symbolTable[i].label, lLabel) == 0)
		{
			if((symbolTable[i].address- address)/2 < -256 | (symbolTable[i].address- address)/2 > 255) exit(3);
			return((symbolTable[i].address- address)/2);
		}
	}
	return -999;
}

//Checks if something is in the symbol table but for pc offset
int isInSymbolTable2(char *lLabel, TableEntry symbolTable[], int tableLoc, int address)
{
	for (int i = 0; i < tableLoc; i++)
	{
		if (strcmp(symbolTable[i].label, lLabel) == 0)
		{
			if((symbolTable[i].address- address)/2 < -1024 | (symbolTable[i].address- address)/2 > 1023) exit(3);
			return((symbolTable[i].address- address)/2);
		}
	}
	return -9001;
}

int isInSymbolTable3(char *lLabel, TableEntry symbolTable[], int tableLoc, int address)
{
	for (int i = 0; i < tableLoc; i++)
	{
		if (strcmp(symbolTable[i].label, lLabel) == 0)
		{
			if((symbolTable[i].address- address)/2 < -256 | (symbolTable[i].address- address)/2 > 255) exit(3);
			return(symbolTable[i].address);
		}
	}
	return -999;
}

//Isolates each 4 binary digits
void seperateHex(char *bin, FILE *out)
{
	fputs("x", out);
	char temp[5];
	temp[0] = bin[0]; temp[1] = bin[1]; temp[2] = bin[2]; temp[3] = bin[3]; temp[4] = '\0';
	outputHex(temp, out);
	temp[0] = bin[4]; temp[1] = bin[5]; temp[2] = bin[6]; temp[3] = bin[7]; temp[4] = '\0';
	outputHex(temp, out);
	temp[0] = bin[8]; temp[1] = bin[9]; temp[2] = bin[10]; temp[3] = bin[11]; temp[4] = '\0';
	outputHex(temp, out);
	temp[0] = bin[12]; temp[1] = bin[13]; temp[2] = bin[14]; temp[3] = bin[15]; temp[4] = '\0';
	outputHex(temp, out);
	fputs("\n", out);
}

//Outputs Hex characters
void outputHex(char *digit, FILE *out)
{
	if(strcmp(digit, "0000") == 0) fputs("0", out);
	else if(strcmp(digit, "0001") == 0) fputs("1", out);
	else if(strcmp(digit, "0010") == 0) fputs("2", out);
	else if(strcmp(digit, "0011") == 0) fputs("3", out);
	else if(strcmp(digit, "0100") == 0) fputs("4", out);
	else if(strcmp(digit, "0101") == 0) fputs("5", out);
	else if(strcmp(digit, "0110") == 0) fputs("6", out);
	else if(strcmp(digit, "0111") == 0) fputs("7", out);
	else if(strcmp(digit, "1000") == 0) fputs("8", out);
	else if(strcmp(digit, "1001") == 0) fputs("9", out);
	else if(strcmp(digit, "1010") == 0) fputs("A", out);
	else if(strcmp(digit, "1011") == 0) fputs("B", out);
	else if(strcmp(digit, "1100") == 0) fputs("C", out);
	else if(strcmp(digit, "1101") == 0) fputs("D", out);
	else if(strcmp(digit, "1110") == 0) fputs("E", out);
	else if(strcmp(digit, "1111") == 0) fputs("F", out);
}



//Appends immediate
void appendImmediate(char *bin, char *lLabel)
{
	char bino[6] = "hello";
	toBinary(toNum(lLabel), 5, bino);
	strcat(bin, bino);
}

//Appends amount
void appendAmount(char *bin, char *lLabel)
{
	char bino[5] = "hell";
	toBinary(toNum(lLabel), 4, bino);
	strcat(bin, bino);
}

//Appends offset
void appendOffset(char *bin, char *lLabel)
{
	char bino[7] = "hellol";
	toBinary(toNum(lLabel), 6, bino);
	strcat(bin, bino);
}

//Appends trap vector
void appendTrapVector(char *bin, char *lLabel)
{
	char bino[9] = "squirrel";
	toBinary(toNum(lLabel), 8, bino);
	strcat(bin, bino);
}

//Converts to a binary char array
void toBinary(int num, int digits, char* result)
{
	int temp = 0;
	int index = digits - 1;
	if (num >= 0)
	{
		while (num > 0)
		{
			temp = num % 2;
			num /= 2;
			if (temp == 0) result[index] = '0';
			else result[index] = '1';
			index--;
		}
		while (index >= 0)
		{
			result[index] = '0';
			index--;
		}
	}
	else
	{
		num = abs(num);
		while (num > 0)
		{
			temp = num % 2;
			num /= 2;
			if (temp == 0) result[index] = '0';
			else result[index] = '1';
			index--;
		}
		while (index >= 0)
		{
			result[index] = '0';
			index--;
		}
		twosComplement(result, digits);
	}
}

//Finds twos complement
void twosComplement(char *in, int digits)
{
	for (int i = 0; i < digits; i++)
	{
		if (in[i] == '0')
		{
			in[i] = '1';
		}
		else in[i] = '0';
	}
	char carry = '1';
	int index = digits - 1;
	while (carry != '0' & index >= 0)
	{
		if (in[index] == '1')
		{
			carry = '1';
			in[index] = '0';
		}
		else
		{
			carry = '0';
			in[index] = '1';
		}
		index--;
	}
}

//Code to add register in binary
void appendRegister(char *bin, char *lLabel)
{
	if (strcmp(lLabel, "r0") == 0) strcat(bin, "000");
	if (strcmp(lLabel, "r1") == 0) strcat(bin, "001");
	if (strcmp(lLabel, "r2") == 0) strcat(bin, "010");
	if (strcmp(lLabel, "r3") == 0) strcat(bin, "011");
	if (strcmp(lLabel, "r4") == 0) strcat(bin, "100");
	if (strcmp(lLabel, "r5") == 0) strcat(bin, "101");
	if (strcmp(lLabel, "r6") == 0) strcat(bin, "110");
	if (strcmp(lLabel, "r7") == 0) strcat(bin, "111");
}

//Code to test the symbol table
void printSymbolTable(TableEntry symbolTable[], int tableLoc)
{
	for (int i = 0; i < tableLoc; i++)
	{
		printf("%s : %i\n", symbolTable[i].label, symbolTable[i].address);
	}
}

//Checks if a function is empty by spaces
int onlySpaces(char *spaces)
{
	if(strlen(spaces) == 0) return 1;
	for (int i = 0; i < strlen(spaces); i++)
	{
		if(spaces[i] != ' ') return 0;
	}
	return 1;
}

//Checks whether a label has already been used
int isDuplicate(char *labelo, TableEntry symbolTable[], int tableLoc, int tracker)
{
	if (tableLoc == tracker) return 0; //reached end of area of importance
	if (strcmp(symbolTable[tracker].label, labelo) == 0) return 1; //found duplicate
	else return(isDuplicate(labelo, symbolTable, tableLoc, tracker + 1)); //next
}

int isValidRegister(char *lLabel)
{
	if (strcmp(lLabel, "r0") == 0) return 1;
	if (strcmp(lLabel, "r1") == 0) return 1;
	if (strcmp(lLabel, "r2") == 0) return 1;
	if (strcmp(lLabel, "r3") == 0) return 1;
	if (strcmp(lLabel, "r4") == 0) return 1;
	if (strcmp(lLabel, "r5") == 0) return 1;
	if (strcmp(lLabel, "r6") == 0) return 1;
	if (strcmp(lLabel, "r7") == 0) return 1;
	return 0;
}

int isValidImmediate(char *lLabel)
{
	int number = toNum(lLabel);
	if (number < -16 | number > 15) exit(3);
	else return 1;
}

int isValidAmount(char *lLabel)
{
	int number = toNum(lLabel);
	if (number < 0 | number > 15) exit(3);
	else return 1;
}

int isValidOffset(char *lLabel)
{
	int number = toNum(lLabel);
	if (number < -32 | number > 31) exit(3);
	else return 1;
}

int isValidTrapVector(char *lLabel)
{
	int number = toNum(lLabel);
	if (number < 0 | number > 255) exit(3);
	else return 1;
}

//Checks if the label is valid
int isValidLabel(char *lLabel)
{
	if (strlen(lLabel) > 20) return 0; //Label is too long
	if (*lLabel == 'x') return 0; //First character can't be x
	if (isAlphanumeric(lLabel) == 0) return 0; //Label must be alphanumeric
	if (strcmp(lLabel, "in") == 0) return 0; //Can't be these words
	if (strcmp(lLabel, "out") == 0) return 0;
	if (strcmp(lLabel, "puts") == 0) return 0;
	if (strcmp(lLabel, "getc") == 0) return 0;
	if (strcmp(lLabel, "r0") == 0) return 0;
	if (strcmp(lLabel, "r1") == 0) return 0;
	if (strcmp(lLabel, "r2") == 0) return 0;
	if (strcmp(lLabel, "r3") == 0) return 0;
	if (strcmp(lLabel, "r4") == 0) return 0;
	if (strcmp(lLabel, "r5") == 0) return 0;
	if (strcmp(lLabel, "r6") == 0) return 0;
	if (strcmp(lLabel, "r7") == 0) return 0;
	return 1;
}

//Checks if an input is alphanumeric
int isAlphanumeric(char *label)
{
	int length = strlen(label); //Find length of input
	for (int i = 0; i < length; i++)
	{
		char temp = *(label + i);
		if ((temp >= 48 & temp <= 57) | (temp >= 97 & temp <= 122)) continue;
		return 0;
	}
	return 1;
}


//Converts a char to a int
int toNum(char * pStr)
{
  	char * t_ptr;
  	char * orig_pStr;
  	int t_length,k;
  	int lNum, lNeg = 0;
  	long int lNumLong;

  	orig_pStr = pStr;
  	if(*pStr == '#')                                /* decimal */
  {
    pStr++;
    if( *pStr == '-' )                                /* dec is negative */
    {
      lNeg = 1;
      pStr++;
    }
    t_ptr = pStr;
    t_length = strlen(t_ptr);
    for(k=0;k < t_length;k++)
    {
      if (!isdigit(*t_ptr))
      {
         printf("Error: invalid decimal operand, %s\n",orig_pStr);
         exit(4);
      }
      t_ptr++;
    }
    lNum = atoi(pStr);
    if (lNeg)
      lNum = -lNum;

    return lNum;
  }
  else if( *pStr == 'x' )        /* hex     */
  {
    pStr++;
    if( *pStr == '-' )                                /* hex is negative */
    {
      lNeg = 1;
      pStr++;
    }
    t_ptr = pStr;
    t_length = strlen(t_ptr);
    for(k=0;k < t_length;k++)
    {
      if (!isxdigit(*t_ptr))
      {
         printf("Error: invalid hex operand, %s\n",orig_pStr);
         exit(4);
      }
      t_ptr++;
    }
    lNumLong = strtol(pStr, NULL, 16);    /* convert hex string into integer */
    lNum = (lNumLong > INT_MAX)? INT_MAX : lNumLong;
    if( lNeg )
      lNum = -lNum;
    return lNum;
  }
  else
  {
        printf( "Error: invalid operand, %s\n", orig_pStr);
        exit(4);  /* This has been changed from error code 3 to error code 4, see clarification 12 */
  }
}
