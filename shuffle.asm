	.ORIG x3000
	LEA R1, ADDR ;Load address of where x3050 is stored into R1
	LDW R1, R1, #0 ;Load x3050 into R1
LOOP	LDB R2, R1, #0 ;Load first byte
	LDB R3, R1, #1 ;Load second byte
	STB R3, R1, #0 ;Swap
	STB R2, R1, #1
	ADD R1, R1, #2 ;Increment memory address
	ADD R0, R0, #-1 ;Decrement size
	BRP LOOPA
ADDR	.FILL x3050
	.END
