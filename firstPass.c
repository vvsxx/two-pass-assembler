#include "header.h"


list *labelLine (int *DC, int *IC, char *token, list *labels_last, list *labels_head, opcode_table *opcodes, int lineNum, int *isCorrect);
int stringLine(char *token, int *isCorrect, int lineNum);
int getAddressLine(char *token, opcode_table *opcodes, int opcode, int lineNum);


void firstPass(char *fileName, image *img, list  *symbols, opcode_table *opcodes){
    list *tmp, *symbols_head = symbols, *entries_head = NULL, *entries_list = NULL; /* list processing */
    char line[LINE_LENGTH], *buffer, *token; /* line processing */
    int IC = 100, DC = 0, lineNum = 0; /* counters */
    int *pIC = &IC, *pDC = &DC;
    int isCorrect = 1; /* errors flag */
    int *p_isCorrect = &isCorrect;
    int opcode; /* opcode decimal value */
    FILE *input;
    strcat(fileName, ".am");
    input = openFile(fileName, "r");
    /* read each line from .am file */
    while (fgets(line, sizeof(line), input) != NULL){
        lineNum++;
        buffer = safeMalloc(sizeof (line));
        strcpy(buffer,line);

        /* separate by tokens */
        line[strcspn(line, "\n")] = '\0';
        token = strtok(line, " ,\t");
        if (token[strlen(token)-1] == ':'){ /* label declaration case */
            symbols = labelLine(pDC, pIC, token, symbols, symbols_head, opcodes, lineNum, p_isCorrect);
        } else if (strcmp(token,".define") == 0) { /* constant definition case */
            symbols = createSymbol(symbols, token, buffer, "mdefine");
        } else if ((opcode = getOpcode(opcodes, token)) != UNKNOWN_OPERATOR){
            IC += getAddressLine(token, opcodes, opcode, lineNum);
            IC++;
        } else if (strcmp(token,".extern") == 0) { /* extern symbol definition case */
            token = strtok(NULL, " \t");
            entries_list = createSymbol(entries_list, token, token, "extern");
            symbols = createSymbol(symbols, token, buffer, "extern");
        } else if (strcmp(token,".entry") == 0) { /* entry symbol definition case */
            token = strtok(NULL, " \t");
            entries_list = createSymbol(entries_list, token, token, "entry");
        } else if (token[0] == ';'){ /* comment case */
            free(buffer);
            continue;
        } else { /* Illegal case */
            printError(UNKNOWN_OPERATOR,lineNum);
            isCorrect = 0;
        }
        if (entries_head == NULL)
            entries_head = entries_list;
        free(buffer);
    }

    if (entries_list != NULL)
        entries_list->next = NULL;
    symbols = symbols_head;
    /* add up counters */
    tmp = symbols_head;
    while (tmp != NULL){
        if (strcmp(tmp->type, "data") == 0)
            tmp->value += IC;
        tmp = tmp->next;
    }
    isCorrect = createEntries(symbols_head, entries_head, fileName);
    fclose(input);
}

int getAddressingMode (char *operand){
    OperandType type;
    operand = deleteWhiteSpaces(operand);
    type = getOpType(operand);
    /* check for empty operator */
    if (strchr(operand,' ') != NULL) {
        return MISSING_COMMA;
    }
    /* get mode */
    if (operand[0] == '\0')
        return UNKNOWN_OPERATOR;
    if (type == REG_DOES_NOT_EXIST)
        return REG_DOES_NOT_EXIST;
    if (type == REGISTER) { /* register */
        return REGISTER_MODE;
    } else if (type == IMMEDIATE_OP){ /* immediate */
        return IMMEDIATE;
    } else if (type == LABEL_OP){ /* direct */
        return DIRECT_MODE;
    } else if (type == ARRAY_INDEX) { /* indexed */
        return INDEXED_MODE;
    }
    return UNKNOWN_OPERAND;
}

int stringLine(char *token, int *isCorrect, int lineNum){
    int DC = 1;
    if (token[0] != '\"'){
        (*isCorrect) = 0;
        printError(ILLEGAL_STRING_DATA, lineNum);
    }
    token++;
    while (strlen(token) > 1){
        DC++;
        token++;
    }
    if (token[0] != '\"'){
        (*isCorrect) = 0;
        printError(ILLEGAL_STRING_DATA, lineNum);
    }
    return DC;
}

int createEntries(list *labels, list *entries, char *fileName){
    int isCorrect = 1;
    list *tmp, *head = entries;
    char ent_fileName[strlen(fileName)+1], ext_fileName[strlen(fileName)+1];
    FILE *ent = NULL, *ext = NULL;
    strncpy(ent_fileName, fileName, strlen(fileName) - 2);
    strncpy(ext_fileName, fileName, strlen(fileName) - 2);
    strcpy(strrchr(ent_fileName, '.'), ".ent");
    strcpy(strrchr(ext_fileName, '.'), ".ext");

    while (entries != NULL){
        if (strcmp(entries->type, "extern") == 0)
            entries->value = 0;
        if (strcmp(entries->type, "entry") == 0) {
            if ((tmp = search_by_name(labels, entries->name)) != NULL)
                entries->value = tmp->value;
            else {
                printError(UNDEFINED_ENTRY, 0); /* lineNum!!!! */
                isCorrect = 0;
            }
        }
        entries = entries->next;
    }
    entries = head;
    while (entries != NULL){
        if (strcmp(entries->type, "entry") == 0) {
            if (ent == NULL)
                ent = openFile(ent_fileName, "w");
            fprintf(ent, "%s\t%.4d\n",entries->name,entries->value);
        }
        if (strcmp(entries->type, "extern") == 0) {
            if (ext == NULL)
                ext = openFile(ext_fileName, "w");
            fprintf(ext, "%s\t%.4d\n",entries->name,entries->value);
        }
        entries = entries->next;
    }
    if (ent != NULL)
        fclose(ent);
    if (ext != NULL)
        fclose(ext);
    return isCorrect;
}

list *labelLine (int *DC, int *IC, char *token, list *labels_last, list *labels_head, opcode_table *opcodes, int lineNum, int *isCorrect){
    int opcode;
    char *labelName;
    labelName = strDuplicate(token);
    labelName[strlen(labelName)-1] = '\0';
    if (search_by_name(labels_head, labelName) != NULL) {
        printError(MULTIPLE_LABEL, lineNum);
        (*isCorrect) = 0;
    }
    labels_last = createSymbol(labels_last, labelName, labelName, "label");
    token = strtok(NULL, " ,\t");
    if (strcmp(token, ".data") == 0){
        labels_last->type = strDuplicate("data");
        labels_last->value =  *DC;
        while ((token = strtok(NULL, ",")) != NULL){
            (*DC)++;
        }
    } else if (strcmp(token, ".string") == 0){
        labels_last->type = strDuplicate("data");
        labels_last->value = *DC;
        token = strtok(NULL, " \t");
        (*DC) += stringLine(token, isCorrect, lineNum);
    } else if ((opcode = getOpcode(opcodes, token)) != UNKNOWN_OPERATOR) { /* opcode found */
        labels_last->type = strDuplicate("code");
        labels_last->value = *IC; /* адрес строки оператора */
        (*IC) += getAddressLine(token, opcodes, opcode, lineNum);
        (*IC)++;
    } else { /* UNKNOWN COMMAND */
        printError(UNKNOWN_OPERATOR, lineNum);
        (*isCorrect) = 0;
    }
    free(labelName);
    return labels_last;
}

/* returns opcode value if exist or UNKNOWN_OPERATOR if doesn't */
int getOpcode(opcode_table *opcodes, char *token){
    int i;
    for (i = 0; i < MAX_OPERATORS; ++i) {
        if (strcmp(token, opcodes->name[i]) == 0)
            return i;
    }
    return UNKNOWN_OPERATOR;
}

/*  */
int getAddressLine(char *token, opcode_table *opcodes, int opcode, int lineNum){
    int j, addressingMode, (*allowed_modes)[MAX_MODES], isReg = 0, IC = 0;
    for (j = 1; (token = strtok(NULL, ",")) != NULL; j++) { /* 'j' is operand number */
        if (j > opcodes->max_ops[opcode])
            printError(EXTRANEOUS_TEXT, lineNum);
        if (j == 1)
            isReg = (token[0] == 'r' && isdigit(token[1]));
        addressingMode = getAddressingMode(token);
        if (addressingMode < 0) { /* ERROR */
            printError(addressingMode, lineNum);
            return 0;
        }
        allowed_modes = (j == 1 && opcodes->max_ops[opcode] > 1 ? opcodes->allowed_src : opcodes->allowed_dst);
        if (!IS_ALLOWED(allowed_modes, opcode, addressingMode))
            printError(ILLEGAL_OPERAND, lineNum);
        if (addressingMode == IMMEDIATE || addressingMode == DIRECT_MODE)
            IC++;
        else if (addressingMode == INDEXED_MODE)
            IC += 2;
        else if (addressingMode == REGISTER_MODE) {
            IC++;
            IC -= j != (isReg && (token[0] == 'r' && isdigit(token[1])));
        } else {
            printError(UNKNOWN_OPERAND, lineNum);
        }
    }
    return IC;
}