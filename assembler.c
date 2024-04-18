#include "header.h"

void test(mem_img *img);

int main(int argc, char *argv[]) {
    int i, errorCode;
    opcode_table *opcodes; /* contains binary values and allowed addressing modes */
    list *symbols; /* contains labels and constants */
    mem_img *img; /* contains binary code */

    /* check that at least one argument had been received */

    
    if (argc < 2) {
        printf("too few arguments, please enter al least 1 filename\n");
        return 1;
    }

    opcodes = safeMalloc(sizeof (opcode_table)); /* create opcodes table */
    getOpTable(opcodes, MAX_REGISTERS); /* fill opcodes table */

    /* run assembler for each file */
    for (i = 1; i < argc; ++i) {
        img = safeMalloc(sizeof (struct memory_image));
        symbols = safeMalloc(sizeof (struct list)); /* create labels list */
        img->IC = 100;
        img->DC = 0;

        if((errorCode = preProcessor(argv[i], opcodes)) == SUCCESS) { /* deploy macros and create ".am" file */
            if ((errorCode = firstPass(argv[i], symbols, opcodes, MEMORY_SIZE)) == SUCCESS) {  /* fill data tables and code mem_img */
                if (secondPass(argv[i], img, opcodes, symbols) == SUCCESS) {  /* convert to binary than to base4 secure and write files */
                    writeFiles(symbols, img, argv[i]);
                }
            }
        }


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

void printError(ErrorCode errorCode, int line){
    switch (errorCode) {
        case UNKNOWN_OPERAND:
            fprintf(stdout,"Unknown operand in line %d\n", line); break;
        case UNKNOWN_OPERATOR:
            fprintf(stdout,"Unknown operator in line %d\n", line); break;
        case ILLEGAL_OPERAND:
            fprintf(stdout,"Illegal operand in line %d\n", line); break;
        case NOT_A_NUMBER:
            fprintf(stdout,"Value is not a number in line %d\n", line); break;
        case EXTRANEOUS_TEXT:
            fprintf(stdout,"Extraneous text in line %d\n", line); break;
        case UNKNOWN_REGISTER:
            fprintf(stdout,"Unknown register in line %d\n", line); break;
        case ILLEGAL_STRING_DATA:
            fprintf(stdout,"Illegal character in line %d\n", line); break;
        case MISSING_COMMA:
            fprintf(stdout,"Missing comma in line %d\n", line); break;
        case REG_DOES_NOT_EXIST:
            fprintf(stdout,"Register does not exist in line %d\n", line); break;
        case UNDEFINED_ENTRY:
            fprintf(stdout,"Undefined entry in line %d\n", line); break;
        case ILLEGAL_LABEL_NAME:
            fprintf(stdout,"Illegal label name in line %d\n", line); break;
        case MULTIPLE_LABEL:
            fprintf(stdout,"The label name repeats an existing one in line %d\n", line); break;
        case TOO_LONG_NAME:
            fprintf(stdout,"Too long label name in line %d\n", line); break;
        case OUT_OF_MEMORY:
            fprintf(stdout,"Not enough memory - %d / 4096\n", line); break;
        case MISSING_OPERAND:
            fprintf(stdout,"Missing operand in line - %d\n", line); break;
        case ILLEGAL_COMMA:
            fprintf(stdout,"Illegal comma in line - %d\n", line); break;
        case ILLEGAL_MACRO_NAME:
            fprintf(stdout,"Illegal macro name in line - %d\n", line); break;
        default:
            fprintf(stdout,"Unknown error in line %d\n", line);
    }
}

/* contains static data to fill opcode table in start of program */
void getOpTable(opcode_table *table, int max_registers) {
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
    memcpy(table->max_ops, operands_needed, sizeof(operands_needed));
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