#include "header.h"

word * createWordNode(struct word *node, int addr);
int getOpValue (char *op, list *symbols);
void codeWords(char *op, image *img, list *symbols, int pos);
void processDataDirective(char *token, image *img, list *symbols);
void createOpValueLine(char *op, image *img, list *symbols, int pos);
char * processOperand(char *op, image *img, int pos);
word * createInstructionWord(image *img);

void secondPass(char *fileName, image *img, opcode_table *op_table, list *symbols){
    struct word *tmp;
    char line[LINE_LENGTH], *token; /* line processing */
    char *src, *dst; /* operands */
    int src_val, dst_val; /* operand values */
    int opcode; /* opcode decimal value */
    int lineNum = 0; /* counters */
    SentenceType sentence;
    FILE *input;
    input = openFile(fileName, "r");
    while (fgets(line, sizeof(line), input) != NULL) {
        lineNum++;
        line[strcspn(line, "\n")] = '\0';
        token = strtok(line, " ,\t");
        sentence = getSentence(op_table, token);
        /* skip label declaration */
        if (sentence == LABEL) {
            token = strtok(NULL, " \t");
            sentence = getSentence(op_table, token);
        }
        /* opcode defined */
        if (sentence == INSTRUCTION) {
            src = NULL;
            dst = NULL;
            /* create new word */
            img->code = createInstructionWord(img);
            resetBits(img->code->binary, WORD_L); /* turn off all bits in word */
            /* set addressing mode bits and get operand tokens */
            opcode = getOpcode(op_table, token);
            if (op_table->max_ops[opcode] == 1) { /* only one operand is allowed */
                dst = processOperand(dst, img, DST_POS);
            } else if (op_table->max_ops[opcode] == 2) { /* two operands allowed */
                src = processOperand(src, img, SRC_MODE_POS); /* code src addressing mode */
                dst = processOperand(dst, img, DST_POS); /* code dst addressing mode */
            }
            /* code opcode bits */
            decimalToBinary(opcode, &img->code->binary[OPCODE_START], OPCODE_LENGTH);
            img->IC++;
            /* create new words for each operand */
            if (src != NULL && dst != NULL) {
                /* both operands are registers and can be coded in one word */
                if (getAddressingMode(src) == REGISTER_MODE && getAddressingMode(dst) == REGISTER_MODE) {
                    img->code = createWordNode(img->code, img->IC);
                    resetBits(img->code->binary, WORD_L);
                    src_val = getOpValue(src, symbols);
                    dst_val = getOpValue(dst, symbols);
                    decimalToBinary(src_val, &img->code->binary[SRC_REG_POS], 3); /* code src bits */
                    decimalToBinary(dst_val, &img->code->binary[DST_POS], 3); /* code dst bits */
                    img->IC++;
                } /* operands are different */
                else {
                    /* src operand coding */
                    createOpValueLine(src, img, symbols, SRC_MODE_POS);
                    /* dst operand coding */
                    createOpValueLine(dst, img, symbols, DST_POS);
                }
            } /* only destination operand */
            else if (dst != NULL){
                createOpValueLine(dst, img, symbols, DST_POS);
            }
            /* data declaration found */
        } else if (sentence == DATA || sentence == STRING) {
            processDataDirective(token, img, symbols);
        }
    }
    img->IC = img->code->address;
    /* add up counters */
    tmp = img->data_h;
    while (tmp != NULL){
        tmp->address += img->IC;
        tmp = tmp->next;
    }
    /* create encrypted base 4 values */
    cryptWord(img->code_h);
    cryptWord(img->data_h);

    fclose(input);
}


/* codes a certain number of words depending on the operand and addressing mode */
void codeWords(char *op, image *img, list *symbols, int pos){
    int *word = img->code->binary;
    int addr_mode, ARE, value;
    char *label, *index;
    list *symbol;
    addr_mode = getAddressingMode(op);
    /* operand is address of label */
    if (addr_mode == DIRECT_MODE) {
        pos = DST_POS;
        symbol = search_by_name(symbols, op);
        ARE = strcmp(symbol->type, "extern") == 0 ? 1 : 2;
        value = symbol->value;
        decimalToBinary(value, &word[DATA_WORD_POS], OP_WORD_L);
        decimalToBinary(ARE, word, 2);
    } else if (addr_mode == IMMEDIATE) { /* # */
        op++;
        if (isalpha(op[0])){ /* value is symbol */
            if ((symbol = search_by_name(symbols, &op[0])) != NULL){
                value = symbol->value;
            }
        } else {
            value = atoi(op);
        }
        decimalToBinary(value, &word[DATA_WORD_POS], OP_WORD_L);
    } else if (addr_mode == REGISTER_MODE) { /* register */
        if (pos == SRC_MODE_POS) /* register src position is different */
            pos = SRC_REG_POS;
        value = atoi(&op[1]);
        decimalToBinary(value, &word[pos], 4);

    } else if (addr_mode == INDEXED_MODE){ /* LABEL[x] */
        label = strdup(op); /* get array name */
        index = strchr(label, '['); /* get array index */
        index[strlen(index) - 1] = '\0'; /* clean */
        index[0] = '\0'; /* clean */
        index++; /* clean */
        symbol = search_by_name(symbols, label);
        value = getOpValue(label, symbols); /* get array address */
        decimalToBinary(value, &word[DATA_WORD_POS], OP_WORD_L); /* code address of label (indexed array) */
        ARE = strcmp(symbol->type, "extern") == 0 ? 1 : 2; /* set ARE */
        decimalToBinary(ARE, word, 2); /* code ARE */
        img->IC++;
        /* create next word */
        img->code = createWordNode(img->code, img->IC);
        word = img->code->binary;
        resetBits(img->code->binary, WORD_L);
        value = getOpValue(index, symbols); /* get index value */
        decimalToBinary(value, &word[DATA_WORD_POS], OP_WORD_L); /* code index value */
    }
}



void createOpValueLine(char *op, image *img, list *symbols, int pos){
    img->code = createWordNode(img->code, img->IC);
    resetBits(img->code->binary, WORD_L);
    codeWords(op, img, symbols, pos);
    img->IC++;
}

char * processOperand(char *op, image *img, int pos){
    int adr_mode;
    op = strtok(NULL, " \t");
    adr_mode = getAddressingMode(op);
    decimalToBinary(adr_mode, &img->code->binary[pos], 2);
    return op;
}

/* processing data directive line */
void processDataDirective(char *token, image *img, list *symbols) {
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
            decimalToBinary(getOpValue(token, symbols), img->data->binary, WORD_L);
            if (img->data_h == NULL)
                img->data_h= img->data;
        }
    }
}

/* creates new instruction word */
word * createInstructionWord(image *img){
    if (img->code_h == NULL){ /* in case of first word in list set head pointer */
        img->code = createWordNode(img->code, img->IC);
        img->code_h = img->code;
    } else {
        img->IC = img->code->address + 1;
        img->code = createWordNode(img->code, img->IC);
    }
    return img->code;
}