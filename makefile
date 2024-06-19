all: assembler

assembler: assembler.o preprocessor.o firstPass.o secondPass.o sentence.o common.o list.o static_data.o
	gcc assembler.o preprocessor.o firstPass.o secondPass.o sentence.o common.o list.o static_data.o -o assembler

assembler.o: assembler.c header.h structs.h typedef.h
	gcc -Wall -ansi -pedantic -c assembler.c

preprocessor.o: preprocessor.c header.h structs.h typedef.h
	gcc -Wall -ansi -pedantic -c preprocessor.c

firstPass.o: firstPass.c header.h structs.h typedef.h
	gcc -Wall -ansi -pedantic -c firstPass.c

secondPass.o: secondPass.c header.h structs.h typedef.h
	gcc -Wall -ansi -pedantic -c secondPass.c

sentence.o: sentence.c header.h structs.h typedef.h
	gcc -Wall -ansi -pedantic -c sentence.c

common.o: common.c header.h structs.h typedef.h
	gcc -Wall -ansi -pedantic -c common.c

list.o: list.c header.h structs.h typedef.h
	gcc -Wall -ansi -pedantic -c list.c

static_data.o: static_data.c header.h structs.h typedef.h
	gcc -Wall -ansi -pedantic -c static_data.c

clean:
	rm -rf *.o assembler

