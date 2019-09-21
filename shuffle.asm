	.ORIG x3000
	LEA R1, ADDR ;Load address of where x3050 is stored into R1
	LDW R1, R1, #0 ;Load x3050 into R1
LOOP	LDB R2, R1, #0 ;Load first byte
	LDB R3, R1, #1 ;Load second byte
	AND R4, R4, #0 ;Clear R4
	ADD R4, R4, R2 ;Add first byte to R4
	AND R2, R2, #0 ;Clear R2
	ADD R2, R2, R3 ;Swape byte
	AND R3, R3, #0 ;Clear R3
	ADD R3, R3, R4 ;Now they have been switched
	STB R3, R1, #0 ;Swap back
	STB R2, R1, #1
	ADD R1, R1, #2 ;Increment memory address
	ADD R0, R0, #-1 ;Decrement size
	BRP LOOPA
ADDR	.FILL x3050
	.END
