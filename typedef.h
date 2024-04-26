typedef struct macros_list macros_list;
typedef struct opcode_table op_table; /* operators and binary values */
typedef struct list list;
typedef struct memory_image mem_img;
typedef struct word word;

typedef enum {
    SYMBOL_LIST,
    MACROS_LIST,
    WORD_LIST
} ListType;

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
    NUMBER_OP,
    REG_DOES_NOT_EXIST
} OperandType;

/* error codes */
typedef enum {
    INCORRECT = 0,
    UNKNOWN_OPERAND = (-1),
    ILLEGAL_OPERAND = (-2),
    UNKNOWN_OPERATOR = (-3),
    UNDEFINED_ENTRY = (-4),
    MULTIPLE_LABEL = (-5),
    EXTRANEOUS_TEXT = (-6),
    ILLEGAL_STRING_DATA = (-7),
    NOT_A_NUMBER = (-8),
    ILLEGAL_COMMA = (-9),
    FILE_NOT_FOUND = (-10),
    MULTIPLE_CONS_COMMAS = (-11),
    MISSING_COMMA = (-12),
    ILLEGAL_REGISTER = (-13),
    ILLEGAL_LABEL_NAME = (-14),
    TOO_LONG_NAME = (-15),
    EMPTY_FILE = (-16),
    OUT_OF_MEMORY = (-17),
    TOO_LONG_LINE = (-18),
    MISSING_OPERAND = (-19),
    ILLEGAL_MACRO_NAME = (-20),
    EMPTY_LABEL = (-21),
    TOO_BIG_VALUE = (-22),
    ILLEGAL_DEF_DIRECT = (-23),
    ILLEGAL_DATA_DIRECT = (-24),
    NOT_AN_INTEGER = (-25),
    ERROR_OPENING_FILE = (-26),
    UNDEFINED_SYMBOL = (-27)
} ErrorCode;