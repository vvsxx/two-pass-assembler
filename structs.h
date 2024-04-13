#define MAX_OPERATORS 16
#define MAX_REGISTERS 8
#define WORD_L 14

struct macros_list {
    char *name;
    int lines;
    char **data;
    struct macros_list *next;
};

struct list {
    char *name;
    char *type;
    int isEntry;
    int isExternal;
    int value;
    int ARE;
    struct list *next;
};


struct opcode_table {
    char name[MAX_OPERATORS][4];
    int max_ops[MAX_OPERATORS];
    int allowed_src[MAX_OPERATORS][4];
    int allowed_dst[MAX_OPERATORS][4];
    char registerNames[MAX_REGISTERS][3];
};

struct word {
    int *binary; /* binary value */
    char *secure4; /* encrypted base 4 value */
    int address; /* decimal address */
    struct word *next;
};

struct image {
    int IC; /* instruction counter */
    int DC; /* data counter */
    struct word *code, *code_h;
    struct word *data, *data_h;
};