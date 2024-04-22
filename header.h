#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <ctype.h>
#include "structs.h"
#include "typedef.h"
/* system parameters */
#define FIRST_ADDRESS 100 /* addressing starts from this address */
#define MEMORY_SIZE 4096
#define MAX_VALUE 4096

/* lengths */
#define LINE_LENGTH 81
#define LABEL_LENGTH 31
#define MAX_MODES 4
#define OPCODE_LENGTH 4
#define OP_WORD_L 12
#define WORD_L 14
#define ARE_SIZE 2
#define ADDR_MODE_SIZE 2
#define REG_WORD_SIZE 3

/* positioning */
#define OPCODE_START 6
#define DATA_WORD_POS 2
#define SRC_POS 4
#define SRC_REG_POS 5
#define DST_POS 2

/* checks */
#define IS_ALLOWED(modes_table, opcode, mode) (modes_table[opcode][mode] == 1 ? 1 : 0)  /* checks that addressing mode is allowed for this opcode*/
#define IS_REGISTER(num) ((num >= 0 && num <= 7) ? 1 : 0) /* checks that addressing mode is allowed for this opcode*/
#define SUCCESS 1

/* addressing modes */
#define IMMEDIATE 0
#define DIRECT_MODE 1
#define INDEXED_MODE 2
#define REGISTER_MODE 3

/* A.R.E. */
#define ARE_ABSOLUTE 0
#define ARE_RELOCATABLE 2
#define ARE_EXTERNAL 1


/* general functions */
FILE * openFile(char *fileName, char *mode);
void *safeMalloc(size_t size);
int preProcessor(char *filename, opcode_table *opcodes);
int firstPass(char *fileName, list  *symbols, opcode_table *opcodes, int memory);
int secondPass(char *fileName, mem_img *img, opcode_table *op_table, list *symbols);
void getOpTable(opcode_table *table, int max_registers);
struct list *getElementByName(struct list *listHead, char *string);
struct macros_list *getMacroByName(struct macros_list *listHead, char *string);
void decimalToBinary(int decimal, int *binary, int array_size);
void binaryToEncrypted4(const int *binary, char *result);
list  * createSymbol(struct list  *list, char *token, char *line, SentenceType type);
void printError(ErrorCode errorCode, int line);
int createEntFile(list *labels, char *fileName);
int getOpcode(opcode_table *opcodes, char *token);
int getAddressingMode (char *operand);
void resetBits(int *arr, int size);
char *strDuplicate(const char *src);
void freeList(void *node, ListType type);
SentenceType getSentence(opcode_table *opcodes, char *token, int lineNum);
char *deleteWhiteSpaces(char *token);
int isNumber(const char *token);
OperandType getOpType(char *token);
void cryptWords(word *wrd);
void writeObjFile(mem_img *img, char *filename);
void writeFiles(list *symbols, mem_img *img, char *filename);
void addAddress(int **arr, int *size, int address);
int getOpValue (char *op, list *symbols, int *isCorrect);
int isLegalName(char *name, opcode_table *opcodes);
int isSavedWord(char *name, opcode_table *opcodes);
int checkLine(char *line, opcode_table *opcodes, int lineNum);