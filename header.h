#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <ctype.h>
#include "structs.h"
#include "typedef.h"
/* lengths */
#define LINE_LENGTH 81
#define LABEL_LENGTH 31
#define MAX_MODES 4
#define OPCODE_LENGTH 4
#define OP_WORD_L 12
#define FIRST_ADDRESS 100

/* word  positioning */
#define OPCODE_START 6
#define DATA_WORD_POS 2
#define SRC_MODE_POS 4
#define SRC_REG_POS 5
#define DST_POS 2


/* checks */
#define IS_ALLOWED(modes_table, opcode, mode) (modes_table[opcode][mode] == 1 ? 1 : 0)  /* checks that addressing mode is allowed for this opcode*/
#define IS_REGISTER(num) ((num >= 0 && num <= 7) ? 1 : 0) /* checks that addressing mode is allowed for this opcode*/

/* addressing modes */
#define IMMEDIATE 0
#define DIRECT_MODE 1
#define INDEXED_MODE 2
#define REGISTER_MODE 3

/* A.R.E. */
#define ARE_ABSOLUTE 0
#define ARE_RELOCATABLE 2
#define ARE_EXTERNAL 1


/* functions */
FILE * openFile(char *fileName, char *mode);
void *safeMalloc(size_t size);
int preProcessor(char *filename);
void getOpTable(opcode_table *table);
struct list *getElementByName(struct list *listHead, char *string);
struct macros_list *getMacroByName(struct macros_list *listHead, char *string);
void decimalToBinary(int decimal, int *binary, int array_size);
void binaryToEncrypted4(const int *binary, char *result);
int firstPass(char *fileName, list  *symbols, opcode_table *opcodes);
int secondPass(char *fileName, image *img, opcode_table *op_table, list *symbols);
list  * createSymbol(struct list  *list, char *token, char *line, SentenceType type);
void printError(int errorCode, int line);
int createEntries(list *labels, char *fileName);
int getOpcode(opcode_table *opcodes, char *token);
int getAddressingMode (char *operand);
void resetBits(int *arr, int size);
char *strDuplicate(const char *src);
void freeList(void *node, ListType type);
SentenceType getSentence(opcode_table *opcodes, char *token);
char *deleteWhiteSpaces(char *token);
int isNumber(char *token);
OperandType getOpType(char *token);
void cryptWords(word *wrd);
void writeObject(image *img, char *filename);