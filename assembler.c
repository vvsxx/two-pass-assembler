#include "header.h"

void test(mem_img *img);
char *filename;
/*
 * The main function of the two-pass assembler program.
 * It processes command line arguments, initializes data structures,
 * performs two passes over the source files, and generates output files.
 *
 * Parameters:
 *   argc: The number of command line arguments.
 *   argv: An array of strings containing the command line arguments.
 *
 * Returns:
 *   0 on successful execution, 1 if too few arguments are provided.
 */
int main(int argc, char *argv[]) {
    int i, errorCode;
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
        filename = argv[i];
        img = safeMalloc(sizeof (struct memory_image));
        symbols = safeMalloc(sizeof (struct list)); /* create labels list */
        img->IC = 100;
        img->DC = 0;
        img->data_h = NULL;
        img->code_h = NULL;
        img->code = NULL;
        img->data = NULL;
        errorCode = preProcessor(argv[i], opcodes); /* deploy macros and create ".am" file */
        if (errorCode != SUCCESS)
            continue;

        errorCode = firstPass(argv[i], symbols, opcodes, MEMORY_SIZE);  /* fill data tables and code mem_img */
        errorCode = secondPass(argv[i], img, opcodes, symbols);  /* convert to binary than to base4 secure and write files */
        if (errorCode != SUCCESS)
            continue;

        errorCode = writeFiles(symbols, img, argv[i]); /* create .ent .ext and .obj files*/
        if (errorCode != SUCCESS)
            printError(errorCode,0);

        test(img);

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
        case UNKNOWN_REGISTER:
            fprintf(stdout,"%s: Unknown register in line %d\n", filename, line); break;
        case ILLEGAL_STRING_DATA:
            fprintf(stdout,"%s: Illegal string declaration in line %d\n", filename, line); break;
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
        case ILLEGAL_DEF_DECLAR:
            fprintf(stdout,"%s: Illegal define declaration in line %d\n", filename,line); break;
        case ILLEGAL_DATA_DIRECT:
            fprintf(stdout,"%s: Illegal data declaration in line %d\n", filename,line); break;
        case UNDEFINED_SYMBOL:
            fprintf(stdout,"%s: Undefined symbol in line %d\n", filename,line); break;
        case TOO_LONG_LINE:
            fprintf(stdout,"%s: WARNING: Line %d is longer than %d characters. Extra characters will be ignored.\n", filename,line, LINE_LENGTH); break;
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

void test(mem_img *img){
    int i, counter, int_val, j, error;
    char line[LINE_LENGTH], *addr, *val, tmp[2];
    char encrypted[10];
    FILE *testfile, *encfile;
    word *wrd_tmp = img->code_h;
    testfile = openFile("testfile", "r");
    encfile = openFile("encrypted_test", "r");
    memset(encrypted, '\0', sizeof (encrypted));
    /*  TESTS */
    while (wrd_tmp != NULL){
        error = 0;
        counter = wrd_tmp->address;
        fgets(encrypted, sizeof(encrypted), encfile);
        encrypted[strcspn(encrypted, "\r")] = '\0';
        if (fgets(line, sizeof(line), testfile) != NULL){
            addr = strtok(line, " \t");
            val = strtok(NULL, " \t");
        } else {
            printf("НЕХВАТАЕТ СТРОК\n");
        }
        if (counter != atoi(addr))
            error = 1;
        if (strcmp(encrypted, wrd_tmp->secure4) != 0)
            error = 1;
        printf("%d\t\t%s\t\t", counter, wrd_tmp->secure4);
        j = 0;
        error = 0;
        for (i = WORD_L - 1; i >= 0; --i, j++) {
            tmp[0] = val[j];
            tmp[1] = '\0';
            int_val = atoi( tmp);
            printf("%d ", wrd_tmp->binary[i]);
            if (int_val != wrd_tmp->binary[i])
                error = 1;
        }
        if (error == 0)
            printf(" OK\n");
        else
            printf(" error\n");
        wrd_tmp = wrd_tmp->next;
    }
    wrd_tmp = img->data_h;
    while (wrd_tmp != NULL){
        counter = wrd_tmp->address;
        if (fgets(line, sizeof(line), testfile) != NULL){
            addr = strtok(line, " \t");
            val = strtok(NULL, " \t");
        } else {
            printf("НЕХВАТАЕТ СТРОК\n");
        }
        if (counter != atoi(addr))
            error = 1;
        printf("%d\t\t%s\t\t", counter, wrd_tmp->secure4);
        error = 0;
        j = 0;
        for (i = WORD_L - 1; i >= 0; --i, j++) {
            tmp[0] = val[j];
            tmp[1] = '\0';
            int_val = atoi( tmp);
            printf("%d ", wrd_tmp->binary[i]);
            if (int_val != wrd_tmp->binary[i])
                error = 1;
        }
        if (error == 0)
            printf(" OK\n");
        else
            printf(" error\n");
        wrd_tmp = wrd_tmp->next;
    }
    fclose(testfile);
}