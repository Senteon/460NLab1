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
void firstPass(FILE *in, TableEntry symbolTable[], char *startingAddress);
int isDuplicate(char *labelo, TableEntry symbolTable[], int tableLoc, int tracker);
int isValidLabel(char *lLabel);
int isAlphanumeric(char *label);
int toNum(char * pStr);
void printSymbolTable(TableEntry symbolTable[]);


int main(int argc, char* argv[]) 
{
	TableEntry symbolTable[MAX_SYMBOLS] = {0};
	FILE *infile;
	FILE *outfile;
	char pLine[MAX_LINE_LENGTH + 1] = {0}; //Initialize pointers to be passed
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

//Performs first pass opeartions
void firstPass(FILE *in, TableEntry symbolTable[], char *startingAddress)
{
   char lLine[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1, *lArg2, *lArg3, *lArg4;
   int lRet;
   int programCounter = 0;
   int tableLoc = 0;
   int starting = toNum(startingAddress);
   do
   {
        lRet = readAndParse(in, lLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4);
        if (lRet != DONE && lRet != EMPTY_LINE)
        {
        	if (isOpcode(lArg1) == 1) exit(4);
        	if (lLabel[0] != '\0') //Check if there is a label
        	{
        		if (isValidLabel(lLabel) == 0) exit(4);
        		if (isDuplicate(lLabel, symbolTable, tableLoc, 0) == 0) //Check if the label has already been used
        		{
        			symbolTable[tableLoc].address = starting + programCounter; //add the label
        			strncpy(symbolTable[tableLoc].label, lLabel, MAX_LABEL_LEN); //add the address
        		}
        		else exit(4);
        		tableLoc += 1;
        		//printf("Line's Label: %s\nLine's Opcode: %s\nLine's A1: %s\nLine's A2: %s\nLine's A3: %s\nLine's A4: %s\n\n", lLabel, lOpcode, lArg1, lArg2, lArg3, lArg4);
        	}
        }
        programCounter += 2;
   } 
   while (lRet != DONE);
   //printSymbolTable(symbolTable);
}

//Code to test the symbol table
void printSymbolTable(TableEntry symbolTable[])
{
	for (int i = 0; i < MAX_SYMBOLS + 1; i++)
	{
		if (symbolTable[i].label[0] != '\0')
		{
			printf("%s : %i\n", symbolTable[i].label, symbolTable[i].address);
		}
	}
}

//Checks whether a label has already been used
int isDuplicate(char *labelo, TableEntry symbolTable[], int tableLoc, int tracker)
{
	if (tableLoc == tracker) return 0; //reached end of area of importance
	if (strcmp(symbolTable[tracker].label, labelo) == 0) return 1; //found duplicate
	else return(isDuplicate(labelo, symbolTable, tableLoc, tracker + 1)); //next 
}

//Checks if the label is valid
int isValidLabel(char *lLabel)
{
	if (strlen(lLabel) > 20) return 0; //Label is too long
	if (*lLabel == 'x') return 0; //First character can't be x
	if (isAlphanumeric(lLabel) == 0) return 0; //Label must be alphanumeric
	if ((*lLabel == 'i') && (*(lLabel+1) == 'n') && (*(lLabel+2) == '\0')) return 0;
	if ((*lLabel == 'o') && (*(lLabel+1) == 'u') && (*(lLabel+2) == 't') && (*(lLabel+3) == '\0')) return 0;
	if ((*lLabel == 'g') && (*(lLabel+1) == 'e') && (*(lLabel+2) == 't') && (*(lLabel+3) == 'c') && (*(lLabel+4) == '\0')) return 0;
	if ((*lLabel == 'p') && (*(lLabel+1) == 'u') && (*(lLabel+2) == 't') && (*(lLabel+3) == 's') && (*(lLabel+4) == '\0')) return 0;
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
