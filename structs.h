#define MAX_OPERATORS 16
#define MAX_REGISTERS 8

struct macros_list {
    char *name;                 /* Name of the macro */
    int lines;                  /* Number of lines in the macro definition */
    char **data;                /* Array of strings representing each line of the macro definition */
    struct macros_list *next;   /* Pointer to the next node in the macros list */
};

struct list {
    char *name;             /* Name of the symbol */
    char *type;             /* Type of the symbol can be code/data/mdefine */
    int value;              /* Decimal value associated with the symbol (e.g., address or constant value) */
    int ARE;                /* Attribute defined by the assembler: Absolute, Relative, or External */
    int isEntry;            /* Flag indicating if the symbol is an entry point */
    int isExternal;         /* Flag indicating if the symbol is external */
    int *addresses;         /* Array of addresses where the symbol is used */
    int addresses_size;     /* Size of the addresses array */
    struct list *next;      /* Pointer to the next symbol in the symbol table */
};


struct opcode_table {
    char name[MAX_OPERATORS][4];            /* Operators names */
    int operands_needed[MAX_OPERATORS];     /* Needed number of operands for each instruction */
    int allowed_src[MAX_OPERATORS][4];      /* Allowed addressing modes for source operands */
    int allowed_dst[MAX_OPERATORS][4];      /* Allowed addressing modes for destination operands */
    char registerNames[MAX_REGISTERS][3];   /* Register names */
};

struct word {
    int binary;              /* binary value */
    char *secure4;            /* encrypted base 4 value */
    int address;              /* decimal address */
    struct word *next;        /* Pointer to the next word */
};

struct memory_image {
    int IC;                         /* instruction counter */
    int DC;                         /* data counter */
    struct word *code, *code_h;     /* Pointer to the head of the instruction words list */
    struct word *data, *data_h;     /* Pointer to the head of the data words list */
};