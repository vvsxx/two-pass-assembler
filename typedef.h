typedef struct macros_list macros_list;
typedef struct opcode_table opcode_table; /* operators and binary values */
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
    NUMBER_OP
} OperandType;

/* errors */
typedef enum {
    UNKNOWN_OPERAND = (-1),
    ILLEGAL_OPERAND = (-2),
    UNKNOWN_OPERATOR = (-3),
    UNDEFINED_ENTRY = (-4),
    MULTIPLE_LABEL = (-5),
    EXTRANEOUS_TEXT = (-6),
    ILLEGAL_STRING_DATA = (-7),
    NOT_A_NUMBER = (-8),
    ILLEGAL_COMMA = (-9),
    UNKNOWN_REGISTER = (-10),
    MULTIPLE_CONS_COMMAS = (-11),
    MISSING_COMMA = (-12),
    REG_DOES_NOT_EXIST = (-13),
    ILLEAGL_LABEL_NAME = (-14),
    TOO_LONG = (-15),
    EMPTY_FILE = (-16),
    OUT_OF_MEMORY = (-17)
} ErrorCode;