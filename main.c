#include "header.h"
void test(struct image *img);

int main(int argc, char *argv[]) {
    int i;
    opcode_table *opcodes; /* contains binary values and allowed addressing modes */
    list *symbols; /* contains labels and constants */
    image *img; /* contains binary code */
    /* check that at least one argument had been received */

    
    if (argc < 2) {
        printf("too few arguments, please enter name of file\n");
        return 1;
    }

    opcodes = safeMalloc(sizeof (opcode_table)); /* create opcodes table */
    fillTable(opcodes); /* fill opcodes table */

    /* run assembler for each file */
    for (i = 1; i < argc; ++i) {
        img = safeMalloc(sizeof (struct image));
        symbols = safeMalloc(sizeof (struct list)); /* create labels list */
        img->IC = 100;
        img->DC = 0;

        preProcessor(argv[i]); /* deploy macros and create ".am" file */
        firstPass(argv[i], img, symbols, opcodes);  /* fill data tables and code image */
        secondPass(argv[i], img, opcodes, symbols); /* convert to binary than to base4 secure and write files */


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

void printError(int errorCode, int line){
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
            fprintf(stdout,"Too many operands or extraneous text in line %d\n", line); break;
        case UNKNOWN_REGISTER:
            fprintf(stdout,"Unknown register in line %d\n", line); break;
        case ILLEGAL_STRING_DATA:
            fprintf(stdout,"Illegal character in line %d\n", line); break;
        case MISSING_COMMA:
            fprintf(stdout,"Missing comma in line %d\n", line); break;
        case REG_DOES_NOT_EXIST:
            fprintf(stdout,"Register does not exist in line %d\n", line); break;
        default:
            fprintf(stdout,"Unknown error in line %d\n", line);
    }
}








void test(struct image *img){
    int i, counter, int_val, j, error;
    char line[LINE_LENGTH], *addr, *val, tmp[2];
    FILE *testfile;
    word *wrd_tmp = img->code_h;
    testfile = openFile("testfile", "r");
    /*  TESTS */
    while (wrd_tmp != NULL){
        error = 0;
        counter = wrd_tmp->address;
        if (fgets(line, sizeof(line), testfile) != NULL){
            addr = strtok(line, " \t");
            val = strtok(NULL, " \t");
        } else {
            printf("НЕХВАТАЕТ СТРОК\n");
        }
        if (counter != atoi(addr))
            error = 1;
        printf("%d   ", counter);
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
    wrd_tmp = img->data_h;
    while (wrd_tmp != NULL){
        error = 0;
        counter = wrd_tmp->address;
        if (fgets(line, sizeof(line), testfile) != NULL){
            addr = strtok(line, " \t");
            val = strtok(NULL, " \t");
        } else {
            printf("НЕХВАТАЕТ СТРОК\n");
        }
        if (counter != atoi(addr))
            error = 1;
        printf("%d   ", counter);
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
    /* TESTS */
}