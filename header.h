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


/* errors */
#define UNKNOWN_OPERAND (-1)
#define ILLEGAL_OPERAND (-2)
#define UNKNOWN_OPERATOR (-3)
#define UNDEFINED_ENTRY (-4)
#define MULTIPLE_LABEL (-5)
#define EXTRANEOUS_TEXT (-6)
#define ILLEGAL_STRING_DATA (-7)
#define NOT_A_NUMBER (-8)
#define ILLEGAL_COMMA (-9)
#define UNKNOWN_REGISTER (-10)
#define MULTIPLE_CONS_COMMAS (-11)
#define MISSING_COMMA (-12)
#define REG_DOES_NOT_EXIST (-13)



FILE * openFile(char *fileName, char *mode);
void freeSymList(struct list *p);
void *safeMalloc(size_t size);
void preProcessor(char *filename);
void fillTable(opcode_table *table);
struct list *search_by_name(struct list *listHead, char *string);
struct macros_list *search_macros_by_name(struct macros_list *listHead, char *string);
void dec_to_bin(int decimal, int *binary, int array_size);
void bin_to_4secure(char *binary, char secure4);
void firstPass(char *fileName, image *img, list  *symbols, opcode_table *opcodes);
void secondPass(char *fileName, image *img, opcode_table *op_table, list *symbols);
list  * createSymbol(struct list  *list, char *token, char *line, char *type);
void printError(int errorCode, int line);
int createEntries(list *labels, list *entries, char *fileName);
int getOpcode(opcode_table *opcodes, char *token);
int getAddressingMode (char *operand);
void resetBits(int *arr, int size);
char *strDuplicate(const char *src);
void freeList(void *node, DataType type);
SentenceType getSentence(opcode_table *opcodes, char *token);
char *deleteWhiteSpaces(char *token);
int isNumber(char *token);
OperandType getOpType(char *token);