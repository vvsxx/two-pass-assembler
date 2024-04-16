#include "header.h"

word * createWordNode(struct word *node, int addr);
int getOpValue (char *op, list *symbols);
void codeWords(char *op, mem_img *img, list *symbols, int pos);
void processDataDirective(char *token, mem_img *img, list *symbols);
void codeOpValue(char *op, mem_img *img, list *symbols, int pos);
char * codeAddressingMode(char *op, mem_img *img, int pos);
word * createInstructionWord(mem_img *img);
void addAddress(int **arr, int *size, int address);

int secondPass(char *fileName, mem_img *img, opcode_table *op_table, list *symbols){
    struct word *tmp;
    char line[LINE_LENGTH], *token; /* line processing */
    char amFile[strlen(fileName) + 4]; /* ".am" + '\0' */
    char *src, *dst; /* operands */
    int src_val, dst_val; /* operand values */
    int opcode; /* opcode decimal value */
    int lineNum = 0; /* counters */
    int isCorrect = 1;
    list *symbol;
    strcpy(amFile, fileName);
    strcat(amFile, ".am");
    SentenceType sentence;
    FILE *input;
    input = openFile(amFile, "r");
    while (fgets(line, sizeof(line), input) != NULL) {
        lineNum++;
        line[strcspn(line, "\n")] = '\0';
        token = strtok(line, " ,\t");
        if (token == NULL)
            continue;
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
                dst = codeAddressingMode(dst, img, DST_POS);
            } else if (op_table->max_ops[opcode] == 2) { /* two operands allowed */
                src = codeAddressingMode(src, img, SRC_MODE_POS); /* code src addressing mode */
                dst = codeAddressingMode(dst, img, DST_POS); /* code dst addressing mode */
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
                    codeOpValue(src, img, symbols, SRC_MODE_POS);
                    /* dst operand coding */
                    codeOpValue(dst, img, symbols, DST_POS);
                }
            } /* only destination operand */
            else if (dst != NULL){
                codeOpValue(dst, img, symbols, DST_POS);
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
    list *symbol;
    addr_mode = getAddressingMode(op);
    /* operand is address of label */
    if (addr_mode == DIRECT_MODE) {
        pos = DST_POS;
        symbol = getElementByName(symbols, op);
        ARE = symbol->ARE;
        value = symbol->value;
        decimalToBinary(value, &word[DATA_WORD_POS], OP_WORD_L);
        decimalToBinary(ARE, word, 2);
    } else if (addr_mode == IMMEDIATE) { /* # */
        op++;
        if (isalpha(op[0])){ /* value is symbol */
            if ((symbol = getElementByName(symbols, &op[0])) != NULL){
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
        value = getOpValue(label, symbols); /* get array address */
        decimalToBinary(value, &word[DATA_WORD_POS], OP_WORD_L); /* code address of label (indexed array) */
        ARE = symbol == NULL ? ARE_ABSOLUTE : symbol->ARE; /* set ARE */
        decimalToBinary(ARE, word, 2); /* code ARE */
        if ((symbol = getElementByName(symbols, label)) != NULL && symbol->isExternal)
            addAddress(&symbol->addresses, &symbol->addresses_size, img->IC);
        img->IC++;
        /* create next word */
        img->code = createWordNode(img->code, img->IC);
        word = img->code->binary;
        resetBits(img->code->binary, WORD_L);
        value = getOpValue(index, symbols); /* get index value */
        decimalToBinary(value, &word[DATA_WORD_POS], OP_WORD_L); /* code index value */
        if ((symbol = getElementByName(symbols, index)) != NULL && symbol->isExternal)
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
    op = strtok(NULL, " \t");
    adr_mode = getAddressingMode(op);
    decimalToBinary(adr_mode, &img->code->binary[pos], 2);
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
            decimalToBinary(getOpValue(token, symbols), img->data->binary, WORD_L);
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

void addAddress(int **arr, int *size, int address) {
    int newSize;
    newSize = (*size) + 1;
    *arr = realloc(*arr, newSize);
    if (*arr == NULL) {
        return;
    }
    (*size) = newSize;
    (*arr)[newSize - 1] = address;
}

void writeFiles(list *symbols, mem_img *img, char *filename){
    createEntFile(symbols, filename);
    writeObjFile(img, filename);
}

/* writes .obj file */
void writeObjFile(mem_img *img, char *filename){
    FILE *objFile;
    char *newName;
    int IC;
    word *tmp;
    newName = safeMalloc(sizeof (filename) + 4); /* +.ob + \0 */
    strcpy(newName, filename);
    strcat(newName, ".ob\0");
    objFile = openFile(newName, "w");
    IC = img->IC-FIRST_ADDRESS + 1; /* +1 because addressing starts from 0 */
    fprintf(objFile, "%d %d\n", IC, img->DC);
    tmp = img->code_h;
    while (tmp != NULL){
        fprintf(objFile, "%.4d %s\n",tmp->address, tmp->secure4);
        tmp = tmp->next;
    }
    tmp = img->data_h;
    while (tmp != NULL){
        fprintf(objFile, "%.4d %s\n",tmp->address, tmp->secure4);
        tmp = tmp->next;
    }
}
int createEntFile(list *labels, char *fileName){
    int isCorrect = 1, i;
    list *head;
    char ent_fileName[strlen(fileName) + 5], ext_fileName[strlen(fileName) + 5]; /* ".ent/.ext" + '\0' */;
    FILE *ent = NULL, *ext = NULL;
    strcpy(ent_fileName, fileName);
    strcpy(ext_fileName, fileName);
    strcat(ent_fileName, ".ent");
    strcat(ext_fileName, ".ext");
    head = labels;
    while (head != NULL){
        if (head->isExternal) {
            if (ext == NULL)
                ext = openFile(ext_fileName, "w");
            for (i = 0; i < head->addresses_size; ++i) {
                fprintf(ext, "%s\t%.4d\n",head->name, head->addresses[i]);
            }
        }
        if (head->isEntry) {
            if (strcmp(head->type,"entry") != 0) {
                if (ent == NULL)
                    ent = openFile(ent_fileName, "w");
                fprintf(ent, "%s\t%.4d\n",head->name,head->value);
            } else {
                printError(UNDEFINED_ENTRY, 0);
                isCorrect = 0;
            }
        }
        head = head->next;
    }

    if (ent != NULL)
        fclose(ent);
    if (ext != NULL)
        fclose(ext);
    return isCorrect;
}