#include "header.h"

/*
 * The program is a two-pass assembler for translating assembly code into machine code according to the rules defined in the technical specifications
 * The preprocessor function prepares the input assembly file by handling macros and generating an intermediate ".am" file.
 * The first pass involves processing each line of the input assembly file to build symbol tables, check syntax, and calculate memory requirements.
 * The second pass generates the binary code and data section by processing each line of the assembly file, coding instructions and data directives, and handling operand addressing modes.
 * if no errors occurred, the program encrypts the obtained binary code into encrypted base 4.
 * If all stages are completed successfully, the program writes the following files: .ent, .ext, and .ob.
 * If errors are detected during any stage, the program outputs an appropriate error message indicating the location of the error.
 *
 * Author: [Vladimir Vais]
 * Date: [24.04.2024]
 */

static char *filename; /* contains current filename */
/*
 * The main function of the two-pass assembler program.
 * It processes command line arguments, initializes data structures,
 * performs two passes over the source files, and generates output files.
 * Parameters:
 *   argc: The number of command line arguments.
 *   argv: An array of strings containing the command line arguments.
 * Returns:
 *   0 on successful execution, 1 if too few arguments are provided.
 */
int main(int argc, char *argv[]) {
    int i, isCorrect, errorCode;
    op_table *opcodes; /* contains binary values and allowed addressing modes */
    list *symbols; /* contains labels and constants */
    mem_img *img; /* contains binary code */
    /* check that at least one argument had been received */
    if (argc < 2) {
        printf("too few arguments, please enter al least 1 filename\n");
        return 1;
    }
    opcodes = safeMalloc(sizeof (op_table)); /* create opcodes table */
    getOpTable(opcodes, MAX_REGISTERS); /* fill opcodes table */

    /* run assembler for each file */
    for (i = 1; i < argc; ++i) {
        errorCode = SUCCESS;
        isCorrect = SUCCESS;
        filename = argv[i];
        symbols = safeMalloc(sizeof (struct list)); /* create labels list */
        symbols->name = NULL; symbols->type = NULL; symbols->next = NULL;
        img = safeMalloc(sizeof (struct memory_image));
        img->IC = 100; img->DC = 0;
        img->data_h = NULL; img->code_h = NULL;
        img->code = NULL; img->data = NULL;
        errorCode = preProcessor(argv[i], opcodes); /* deploy macros and create ".am" file */
        if (errorCode != SUCCESS && errorCode != FILE_NOT_FOUND) {
            printError(errorCode,0);
            isCorrect = INCORRECT;
        }
        if (errorCode != EMPTY_FILE && errorCode != FILE_NOT_FOUND) {
            errorCode = firstPass(argv[i], symbols, opcodes,
                                  (MEMORY_SIZE - FIRST_ADDRESS));  /* fill data tables and code mem_img */
            if (errorCode != SUCCESS)
                isCorrect = INCORRECT;
            errorCode = secondPass(argv[i], img, opcodes,
                                   symbols);  /* convert to binary than to base4 secure and write files */
            if (errorCode != SUCCESS)
                isCorrect = INCORRECT;

            if (isCorrect == SUCCESS)
                isCorrect = writeFiles(symbols, img, argv[i]); /* create .ent .ext and .obj files*/
            if (isCorrect != SUCCESS)
                printError(INCORRECT, 0);
        }

        /* free memory & close opened files */
        freeList(symbols, SYMBOL_LIST);
        freeList(img->code_h, WORD_LIST);
        freeList(img->data_h, WORD_LIST);
        free(img);

    }
    free(opcodes);
    return 0;
}

/* receives error code and line number and displays a corresponding warning message. */
void printError(ErrorCode errorCode, int line){
    switch (errorCode) {
        case UNKNOWN_OPERAND:
            fprintf(stdout,"%s: Unknown operand in line %d\n", filename, line); break;
        case UNKNOWN_OPERATOR:
            fprintf(stdout,"%s: Unknown operator in line %d\n", filename, line); break;
        case ILLEGAL_OPERAND:
            fprintf(stdout,"%s: Illegal operand in line %d\n", filename, line); break;
        case NOT_A_NUMBER:
            fprintf(stdout,"%s: Value is not a number in line %d\n", filename, line); break;
        case EXTRANEOUS_TEXT:
            fprintf(stdout,"%s: Extraneous text in line %d\n", filename, line); break;
        case ILLEGAL_STRING_DATA:
            fprintf(stdout,"%s: Illegal string directive in line %d\n", filename, line); break;
        case MISSING_COMMA:
            fprintf(stdout,"%s: Missing comma in line %d\n", filename, line); break;
        case ILLEGAL_REGISTER:
            fprintf(stdout,"%s: Register does not exist in line %d\n", filename, line); break;
        case UNDEFINED_ENTRY:
            fprintf(stdout,"%s: Undefined entry in line %d\n", filename,line); break;
        case ILLEGAL_LABEL_NAME:
            fprintf(stdout,"%s: Illegal label name in line %d\n", filename,line); break;
        case MULTIPLE_LABEL:
            fprintf(stdout,"%s: The label name repeats an existing one in line %d\n", filename,line); break;
        case TOO_LONG_NAME:
            fprintf(stdout,"%s: Too long label name in line %d\n", filename,line); break;
        case OUT_OF_MEMORY:
            fprintf(stdout,"%s: Not enough memory - %d / 4096\n", filename,line); break;
        case MISSING_OPERAND:
            fprintf(stdout,"%s: Missing operand in line - %d\n", filename,line); break;
        case ILLEGAL_COMMA:
            fprintf(stdout,"%s: Illegal comma in line - %d\n", filename,line); break;
        case ILLEGAL_MACRO_NAME:
            fprintf(stdout,"%s: Illegal macro name in line - %d\n", filename, line); break;
        case EMPTY_FILE:
            fprintf(stdout,"%s: File is empty\n", filename); break;
        case EMPTY_LABEL:
            fprintf(stdout,"%s: WARNING: Empty label in line %d\n", filename,line); break;
        case MULTIPLE_CONS_COMMAS:
            fprintf(stdout,"%s: Multiple consecutive commas in line %d\n", filename,line); break;
        case ILLEGAL_DEF_DIRECT:
            fprintf(stdout,"%s: Illegal define declaration in line %d\n", filename,line); break;
        case ILLEGAL_DATA_DIRECT:
            fprintf(stdout,"%s: Illegal data directive in line %d\n", filename,line); break;
        case UNDEFINED_SYMBOL:
            fprintf(stdout,"%s: Undefined symbol in line %d\n", filename,line); break;
        case TOO_LONG_LINE:
            fprintf(stdout,"%s: WARNING: Line %d is longer than %d characters. Extra characters will be ignored.\n", filename,line, LINE_LENGTH); break;
        case TOO_BIG_VALUE:
            fprintf(stdout,"%s: WARNING: The value in line %d is more than 12bits. The value must be between %d and %d", filename, line, (MEMORY_SIZE / 2) - 1, ((MEMORY_SIZE / 2) - 1)*(-1)); break;
        case NOT_AN_INTEGER:
            fprintf(stdout,"%s: WARNING: Value is not an integer in line %d\n", filename, line); break;
        case INCORRECT:
            fprintf(stdout,"%s: assembly failed\n", filename); break;
        default:
            fprintf(stdout,"%s: Unknown error in line %d\n", filename,line);
    }
}

/* contains static data to fill opcode table in start of program */
void getOpTable(op_table *table, int max_registers) {
    int i;
    char name[MAX_OPERATORS][OPCODE_LENGTH] = {"mov", "cmp", "add", "sub",
                                               "not", "clr", "lea", "inc",
                                               "dec", "jmp", "bne", "red",
                                               "prn", "jsr", "rts", "hlt"};

    int operands_needed[MAX_OPERATORS] = {2, 2, 2, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 0, 0};
    int alwd_src[MAX_OPERATORS][MAX_MODES] = {{1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1},
                                              {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0},
                                              {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0},
                                              {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};

    int alwd_dst[MAX_OPERATORS][MAX_MODES] = {{0, 1, 1, 1}, {1, 1, 1, 1}, {0, 1, 1, 1}, {0, 1, 1, 1},
                                              {0, 1, 1, 1}, {0, 1, 1, 1}, {0, 1, 1, 1}, {0, 1, 1, 1},
                                              {0, 1, 1, 1}, {0, 1, 0, 1}, {0, 1, 0, 1}, {0, 1, 1, 1},
                                              {1, 1, 1, 1}, {0, 1, 0, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}};

    for (i = 0; i < max_registers; ++i) {
        table->registerNames[i][0] = 'r';
        table->registerNames[i][1] = '0' + i;
    }

    memcpy(table->name, name, sizeof(name));
    memcpy(table->operands_needed, operands_needed, sizeof(operands_needed));
    memcpy(table->allowed_src, alwd_src, sizeof(alwd_src));
    memcpy(table->allowed_dst, alwd_dst, sizeof(alwd_dst));
}
