typedef struct macros_list macros_list;
typedef struct opcode_table opcode_table; /* operators and binary values */
typedef struct list list;
typedef struct image image;
typedef struct word word;

typedef enum {
    SYMBOL_LIST,
    MACROS_LIST,
    WORD_LIST
} DataType;

typedef enum {
    COMMENT,
    EMPTY,
    DEFINE,
    DIRECTIVE,
    ENTRY,
    EXTERN,
    DATA,
    STRING,
    INSTRUCTION,
    LABEL
} SentenceType;

typedef enum {
    REGISTER,
    IMMEDIATE_OP,
    LABEL_OP,
    ARRAY_INDEX,
    NUMBER_OP
} OperandType;
