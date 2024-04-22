#include "header.h"

/* functions and variables accessible only from this file */
word * createWordNode(struct word *node, int addr);
void codeWords(char *op, mem_img *img, list *symbols, int pos);
void processDataDirective(char *token, mem_img *img, list *symbols);
void codeOpValue(char *op, mem_img *img, list *symbols, int pos);
char * codeAddressingMode(char *op, mem_img *img, int pos);
word * createInstructionWord(mem_img *img);
int errorCode; /* accessible only from this file, uses to detect logical problems in multiple functions */

/*
 * Performs the second pass of the assembly process, generating the binary code and data section.
 * It processes each line of the ".am" file, codes instructions and data directives,
 * and handles operand addressing modes.
 * Parameters:
 *   fileName: The name of the source file being processed.
 *   img: Pointer to the memory image structure containing binary code and data.
 *   op_table: Pointer to the opcode table containing binary values and addressing modes.
 *   symbols: Pointer to the list of symbols (labels and constants).
 * Returns:
 *   SUCCESS if the second pass completes without errors, or 0 if an error occurs.
 */
int secondPass(char *fileName, mem_img *img, op_table *op_table, list *symbols){
    struct word *tmp; /* temporary pointer to process words */
    char line[LINE_LENGTH], *token; /* line processing */
    int fileNameSize = strlen(fileName) + 4;
    char amFile[fileNameSize]; /* ".am" + '\0' */
    char *src, *dst; /* operands */
    int src_val, dst_val; /* operand values */
    int opcode; /* opcode decimal value */
    int lineNum = 0; /* counters */
    int isCorrect = SUCCESS;
    SentenceType sentence;
    list *symbol;
    strcpy(amFile, fileName);
    strcat(amFile, ".am");
    FILE *input;
    input = openFile(amFile, "r");
    if (input == NULL) /* can't open input file */
        return INCORRECT;
    while (fgets(line, sizeof(line), input) != NULL) {
        errorCode = SUCCESS;
        lineNum++;
        line[strcspn(line, "\n")] = '\0';
        token = strtok(line, " \t");
        if (token == NULL)
            continue;
        sentence = getSentence(op_table, token, lineNum);
        if (sentence == LABEL) { /* skip label declaration */
            token = strtok(NULL, " \t");
            sentence = getSentence(op_table, token, lineNum);
        }
        if (sentence == INSTRUCTION) {
            src = NULL;
            dst = NULL;
            img->code = createInstructionWord(img); /* create new word */
            resetBits(img->code->binary, WORD_L); /* turn off all bits in word */
            /* set addressing mode bits and get operand tokens */
            opcode = getOpcode(op_table, token);
            if (op_table->operands_needed[opcode] == 1) { /* only one operand is allowed */
                dst = codeAddressingMode(dst, img, DST_POS);
                if (dst == NULL){
                    printError(MISSING_OPERAND, lineNum);
                    isCorrect = INCORRECT;
                    continue;
                }
            } else if (op_table->operands_needed[opcode] == 2) { /* two operands allowed */
                src = codeAddressingMode(src, img, SRC_POS); /* code src addressing mode */
                dst = codeAddressingMode(dst, img, DST_POS); /* code dst addressing mode */
                if (src == NULL || dst == NULL){
                    printError(MISSING_OPERAND, lineNum);
                    isCorrect = INCORRECT;
                    continue;
                }
            }
            decimalToBinary(opcode, &img->code->binary[OPCODE_START], OPCODE_LENGTH); /* code opcode bits */
            img->IC++; /* increase instruction counter */
            /* create new words for each operand */
            if (src != NULL && dst != NULL) {
                /* both operands are registers and can be coded in one word */
                if (getAddressingMode(src) == REGISTER_MODE && getAddressingMode(dst) == REGISTER_MODE) {
                    img->code = createWordNode(img->code, img->IC);
                    resetBits(img->code->binary, WORD_L);
                    src_val = getOpValue(src, symbols, &errorCode);
                    dst_val = getOpValue(dst, symbols, &errorCode);
                    decimalToBinary(src_val, &img->code->binary[SRC_REG_POS], REG_WORD_SIZE); /* code src bits */
                    decimalToBinary(dst_val, &img->code->binary[DST_POS], REG_WORD_SIZE); /* code dst bits */
                    img->IC++;
                } else { /* operands are different */
                    codeOpValue(src, img, symbols, SRC_POS); /* src operand coding */
                    codeOpValue(dst, img, symbols, DST_POS); /* dst operand coding */
                }
            } else if (dst != NULL){ /* only destination operand */
                codeOpValue(dst, img, symbols, DST_POS);
            }
        } else if (sentence == DATA || sentence == STRING) { /* data declaration found */
            processDataDirective(token, img, symbols);
        }
        if (errorCode != SUCCESS) {
            printError(errorCode, lineNum);
            isCorrect = INCORRECT;
        }
    }
    if (img->code != NULL) {
        img->IC = img->code->address;
    }
    /* add up counters */
    tmp = img->data_h;
    while (tmp != NULL){
        tmp->address += img->IC;
        tmp = tmp->next;
    }
    /* create encrypted base 4 values */
    cryptWords(img->code_h);
    cryptWords(img->data_h);
    fclose(input);
    return isCorrect;
}


/* codes a certain number of words depending on the operand and addressing mode */
void codeWords(char *op, mem_img *img, list *symbols, int pos){
    int *word = img->code->binary;
    int addr_mode, ARE, value = 0;
    char *label, *index, *c;
    list *symbol = NULL;
    addr_mode = getAddressingMode(op);
    /* operand is address of label */
    if (addr_mode == DIRECT_MODE) {
        pos = DST_POS;
        symbol = getElementByName(symbols, op);
        if (symbol == NULL) {
            errorCode = UNDEFINED_SYMBOL;
            return;
        }
        ARE = symbol->ARE;
        value = symbol->value;
        decimalToBinary(value, &word[DATA_WORD_POS], OP_WORD_L);
        decimalToBinary(ARE, word, ARE_SIZE);
    } else if (addr_mode == IMMEDIATE) { /* # */
        value = getOpValue(op, symbols, &errorCode);
        decimalToBinary(value, &word[DATA_WORD_POS], OP_WORD_L);
    } else if (addr_mode == REGISTER_MODE) { /* register */
        if (pos == SRC_POS) /* register src position is different */
            pos = SRC_REG_POS;
        value = getOpValue(op, symbols, &errorCode);
        decimalToBinary(value, &word[pos], REG_WORD_SIZE);
    } else if (addr_mode == INDEXED_MODE){ /* LABEL[x] */
        label = strDuplicate(op); /* get array name */
        index = strDuplicate(strchr(label, '[')); /* get array index */
        if (index[strlen(index) - 1] == ',')
            index[strlen(index) - 2] = '\0'; /* clean */
        else
            index[strlen(index) - 1] = '\0'; /* clean */
        index++; /* clean */
        c = strchr(label, '[');
        (*c) = '\0';
        symbol = getElementByName(symbols, label);
        if (symbol == NULL || strcmp(symbol->type, "entry") == 0) {
            errorCode = UNDEFINED_SYMBOL;
            return;
        }
        value = getOpValue(label, symbols, &errorCode); /* get array address */
        decimalToBinary(value, &word[DATA_WORD_POS], OP_WORD_L); /* code address of label (indexed array) */
        ARE = symbol == NULL ? ARE_ABSOLUTE : symbol->ARE; /* set ARE */
        decimalToBinary(ARE, word, ARE_SIZE); /* code ARE */
        if (symbol->isExternal)
            addAddress(&symbol->addresses, &symbol->addresses_size, img->IC);

        img->IC++;
        /* create next word */
        img->code = createWordNode(img->code, img->IC);
        word = img->code->binary;
        resetBits(img->code->binary, WORD_L);
        value = getOpValue(index, symbols, &errorCode); /* get index value */
        decimalToBinary(value, &word[DATA_WORD_POS], OP_WORD_L); /* code index value */
        symbol = getElementByName(symbols, index);
        if ((symbol == NULL && !isNumber(index)) || (symbol != NULL && strcmp(symbol->type, "entry") == 0)) {
            errorCode = UNDEFINED_SYMBOL;
            return;
        }
        if (symbol != NULL && symbol->isExternal)
            addAddress(&symbol->addresses, &symbol->addresses_size, img->IC);

        free(label);
        index--;
        free(index);
    }
    if ((symbol = getElementByName(symbols, op)) != NULL && symbol->isExternal)
        addAddress(&symbol->addresses, &symbol->addresses_size, img->IC);

}


void codeOpValue(char *op, mem_img *img, list *symbols, int pos){
    img->code = createWordNode(img->code, img->IC);
    resetBits(img->code->binary, WORD_L);
    codeWords(op, img, symbols, pos);
    img->IC++;
}

char * codeAddressingMode(char *op, mem_img *img, int pos){
    int adr_mode;
    op = strtok(NULL, " ,");
    if (op == NULL)
        return NULL;
    adr_mode = getAddressingMode(op);
    decimalToBinary(adr_mode, &img->code->binary[pos], ADDR_MODE_SIZE);
    return op;
}

/* processing data directive line */
void processDataDirective(char *token, mem_img *img, list *symbols) {
    list *symbol;
    if (strcmp(token, ".string") == 0) {
        token = strtok(NULL, " ");
        token = strchr(token, '\"');
        token++;
        while (strlen(token) > 1){
            img->DC = img->data != NULL ? img->data->address + 1 : 1;
            img->data = createWordNode(img->data, img->DC);
            decimalToBinary(token[0], img->data->binary, WORD_L);
            if (img->data_h == NULL)
                img->data_h = img->data;
            token++;
        }
        img->DC = img->data->address + 1;
        img->data = createWordNode(img->data, img->DC);
        resetBits(img->data->binary, WORD_L);
    } else if (strcmp(token, ".data") == 0) {
        while ((token = strtok(NULL, ",")) != NULL) {
            img->DC = img->data != NULL ? img->data->address + 1 : 1;
            img->data = createWordNode(img->data, img->DC);
            decimalToBinary(getOpValue(token, symbols, &errorCode), img->data->binary, WORD_L);
            if ((symbol = getElementByName(symbols, token)) != NULL && symbol->isExternal)
                addAddress(&symbol->addresses, &symbol->addresses_size, img->DC + img->IC);
            if (img->data_h == NULL)
                img->data_h= img->data;
        }
    }
}

/* creates new instruction word */
word * createInstructionWord(mem_img *img){
    if (img->code_h == NULL){ /* in case of first word in list set head pointer */
        img->code = createWordNode(img->code, img->IC);
        img->code_h = img->code;
    } else {
        img->IC = img->code->address + 1;
        img->code = createWordNode(img->code, img->IC);
    }
    return img->code;
}