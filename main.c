#include <stdio.h> /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h> /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */

#define MAX_LINE_LENGTH 255
#define MAX_LABEL_LEN 20 //Struct for the Symbol Table
#define MAX_SYMBOLS 255

enum {DONE, OK, EMPTY_LINE};
typedef struct 
{
	int address;
	char label[MAX_LABEL_LEN + 1];	
} TableEntry;


int determineValidHex(char *startingAddress);
int isOpcode(char *inLoc);
int readAndParse(FILE *pInfile, char *pLine, char **pLabel, char **pOpcode, char **pArg1, char **pArg2, char **pArg3, char **pArg4);
void firstPass();


int main(int argc, char* argv[]) 
{
	TableEntry symbolTable[MAX_SYMBOLS];
	FILE *infile;
	FILE *outfile;
	char pLine[MAX_LINE_LENGTH + 1]; //Initialize pointers to be passed
	char *pLabel, *pOpcode, *pArg1, *pArg2, *pArg3, *pArg4;
	char startingAddress[5];

    infile = fopen(argv[1], "r");
    outfile = fopen(argv[2], "w");


    if (!infile)  //Open input file
    {
      printf("Error: Cannot open file %s\n", argv[1]);
      exit(4);
    }
    if (!outfile) //Open output file
    {
      printf("Error: Cannot open file %s\n", argv[2]);
      exit(4);
    }


    // ORIG checks//
    int returnValue = readAndParse(infile, pLine, &pLabel, &pOpcode, &pArg1, &pArg2, &pArg3, &pArg4); //Check for .ORIG
    if (strlen(pLabel) > 0) exit(4); //Make sure pLabel doesn't have anything in it
    if (strcmp(pOpcode, ".orig") != 0) exit(4); //No .ORIG
    if (strlen(pArg1) != 5) exit(3); //Adress is incorrect size 
    strncpy(startingAddress, pArg1, sizeof(startingAddress)); //Find starting address;
    if (determineValidHex(startingAddress) == 0) exit(4); //Incorrect address formatting

    /*First Pass*/
    firstPass(infile, symbolTable, startingAddress);
    

    fclose(infile); //Close files
    fclose(outfile);
    exit(0);
}

int determineValidHex(char *startingAddress)
{
	if (*startingAddress != 'x') return 0;
	for (int i = 1; i < 5; i++)
	{
		char temp = *(startingAddress + i);
		if ((temp >= 48 & temp <= 57) | (temp >= 97 & temp <= 102)) continue;
		return 0;
	}
	char t = *(startingAddress + 4);
	if (t == '1' | t == '3' | t == '5' | t == '7' | t == '9' | t == 'B' | t == 'D' | t == 'F') return 0;
	return 1;
}

//Function to determine if the string is an opcode
int isOpcode(char *inLoc)
{
	/* Declaring all possibilities */                                                                                                                             
	char *list[31] = {"add", "and", "br", "brn", "brz", "brp", "brnz", "brnp", "brnz", "brnzp", "halt", "jmp", "jsr", "jsrr", "ldb", "ldw", "lea", "nop", "not", "ret", "lshf", "rshfl", "rshfa", "rti", "stb", "stw", "trap", "xor", ".orig", ".fill", ".end"};
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

void firstPass(FILE *in, TableEntry symbolTable[], char *startingAddress)
{
   char lLine[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1, *lArg2, *lArg3, *lArg4;
   int lRet;
   int programCounter = 2;
   int tableLoc = 0;

   do
   {
        lRet = readAndParse(in, lLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4);
        if (lRet != DONE && lRet != EMPTY_LINE)
        {
        	if (lLabel != '\0')
        	{
        		if (isDuplicate(lLabel, symbolTable, tableLoc, 0) == 0)
        		{

        		}
        		//printf("Line's Label: %s\nLine's Opcode: %s\nLine's A1: %s\nLine's A2: %s\nLine's A3: %s\nLine's A4: %s\n\n", lLabel, lOpcode, lArg1, lArg2, lArg3, lArg4);
        	}
        }
        programCounter += 2;
   } 
   while (lRet != DONE);
}
int isDuplicate(char *labelo, TableEntry symbolTable[], int tableLoc, int tracker)
{
	if (tableLoc == tracker) return 0;
	if (strcmp(symbolTable[tracker].label, labelo == 0)) return 1;
	else return(isDuplicate(labelo, symbolTable, tableLoc, tracker + 1));
}
