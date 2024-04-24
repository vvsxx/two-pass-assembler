;Unknown operand in line 7
;The label name repeats an existing one in line 9
;Register does not exist in line 10
;Unknown operator in line 14
;Illegal string directive in line 15
;Undefined symbol in line 11
;Illegal data directive in line 17

.extern XYZ
    .entry MAIN

KINITIALVALUE: sub  r4   ,     r3

MAIN:   mov  r3, LENGTH

LOOP:   jmp L1

    mcr M1
sub  r1,  r9
    bne END
    endmcr

    .entry GGG
prn -5
bne LOOP

XYZ: mov  r4,  r2

M1

L1:     inc K
bne LOOP

stopp

STR:    .string "666"abcdef"

LENGTH: .data 6, -9, 15

K:      .data 4    ,  ,  -55,4,4,4,6

    mcr M2
mov reg1, val
add reg2, reg1
    endmcr

M2
ABC: mov XYZ,  r3
reg1: .data 6,5,-555,66
reg2: .data 6,5,-555,66
val: .string "asfas   %%dfjk"