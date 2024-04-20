#include "header.h"

list *processLabel (SentenceType  type, int *DC, int *IC, char *labelName, char *token, list *labels_last, list *labels_head, opcode_table *opcodes, int lineNum, int *isCorrect);
int stringLine(char *token, int *isCorrect, int lineNum);
int processInstruction(char *token, opcode_table *opcodes, int opcode, int lineNum);


int firstPass(char *fileName, list  *symbols, opcode_table *opcodes, int memory){
    list *tmp, *symbols_head = symbols; /* list processing */
    char line[LINE_LENGTH], *buffer, *token; /* line processing */
    char newFileName[strlen(fileName) + 4]; /* ".am" + '\0' */
    char labelName[LABEL_LENGTH];
    int IC = FIRST_ADDRESS, DC = 0, mem_counter = 0, lineNum = 0; /* counters */
    int isCorrect = 1; /* errors flag */
    int *pIC = &IC, *pDC = &DC, *p_isCorrect = &isCorrect; /* pointers */
    int opcode; /* opcode decimal value */
    SentenceType type;
    FILE *input;
    strcpy(newFileName, fileName);
    strcat(newFileName, ".am");
    input = openFile(newFileName, "r");
    buffer = safeMalloc(sizeof (line));
    /* read each line from .am file */
    while (fgets(line, sizeof(line), input) != NULL){
        token = deleteWhiteSpaces(line); /* used to check line correctness */
        isCorrect = checkLine(token);
        if (isCorrect != SUCCESS) {
            printError(isCorrect, lineNum);
            continue;
        }
        memset(labelName, '\0', strlen(labelName));
        lineNum++;
        memset(buffer, '\0', strlen(buffer));
        strcpy(buffer,line);
        line[strcspn(line, "\n")] = '\0';
        token = strtok(line, " \t");
        if (token == NULL)
            continue;
        if (token[strlen(token)-1] == ',') {
            printError(ILLEGAL_COMMA, lineNum);
            isCorrect;
            token[strlen(token)-1] = '\0';
        }
        type = getSentence(opcodes, token);
        if (type == LABEL){ /* label declaration case */
            strcpy(labelName, token);
            if ((token = strtok(NULL, " \t")) == NULL) { /* label before empty line */
                printError(EMPTY_LABEL, lineNum);
                continue;
            }

            type = getSentence(opcodes, token);
            if (type != ENTRY && type != EXTERN && token != NULL)
                symbols = processLabel(type, pDC, pIC, labelName, token, symbols, symbols_head, opcodes, lineNum,
                                       p_isCorrect);
        } else if (type == DEFINE) { /* constant definition case */
            symbols = createSymbol(symbols, token, buffer, type);
        } else if (type == INSTRUCTION){
            opcode = getOpcode(opcodes, token);
            IC += processInstruction(token, opcodes, opcode, lineNum);
            IC++;
        } else if (type == EXTERN) { /* extern symbol definition case */
            token = strtok(NULL, " \t");
            symbols = createSymbol(symbols, token, buffer, type);
        } else if (type == ENTRY) { /* entry symbol definition case */
            token = strtok(NULL, " \t");
            if ((tmp = getElementByName(symbols_head, token)) != NULL)
                tmp->isEntry = 1;
            else
                symbols = createSymbol(symbols, token, token, type);
        } else if (type == COMMENT){ /* comment case */
            continue;
        }
        else if (type == DATA || type == STRING){
            if (type == STRING) {
                token = strtok(NULL, " \t");
                DC += stringLine(token, &isCorrect, lineNum);
            } else {
                while ((token = strtok(NULL, ",")) != NULL)
                    DC++;
            }
        } else { /* Illegal case */
            printError(UNKNOWN_OPERATOR,lineNum);
            isCorrect = INCORRECT;;
        }
    }
    /* add up counters */
    tmp = symbols_head;
    while (tmp != NULL){
        if (strcmp(tmp->type, "data") == 0)
            tmp->value += IC;
        tmp = tmp->next;
    }
    if ((mem_counter = (IC - FIRST_ADDRESS) + DC) > memory) {
        printError(OUT_OF_MEMORY, mem_counter);
        isCorrect = INCORRECT;;
    }
    free(buffer);
    fclose(input);
    return isCorrect;
}


int stringLine(char *token, int *isCorrect, int lineNum){
    int DC = 1;
    if (token[0] != '\"'){
        (*isCorrect) = INCORRECT;
        printError(ILLEGAL_STRING_DATA, lineNum);
    }
    token++;
    while (token[0] != '\"'){
        DC++;
        token++;
    }
    if (token[0] != '\"'){
        (*isCorrect) = INCORRECT;
        printError(ILLEGAL_STRING_DATA, lineNum);
    }
    token = strtok(token, " \" ");
    if (token != NULL){
        (*isCorrect) = INCORRECT;
        printError(EXTRANEOUS_TEXT, lineNum);
    }
    return DC;
}

list *processLabel (SentenceType type, int *DC, int *IC, char *labelName, char *token, list *labels_last, list *labels_head, opcode_table *opcodes, int lineNum, int *isCorrect){
    int opcode;
    int errorCheck;
    list *current, *tmp;
    labelName[strlen(labelName)-1] = '\0';
    if ((errorCheck = isLegalName(labelName, opcodes)) != 0){
        (*isCorrect) = INCORRECT;
        printError(ILLEGAL_LABEL_NAME, lineNum);
    }

    if ((tmp = getElementByName(labels_head, labelName)) != NULL) {
        if (strcmp(tmp->type, "entry") != 0) {
            printError(MULTIPLE_LABEL, lineNum);
            (*isCorrect) = INCORRECT;
        } else {
            current = tmp;
        }
    } else {
        labels_last = createSymbol(labels_last, labelName, labelName, type);
        current = labels_last;
    }

    if (type == DATA || type == STRING){
        if (current->type == NULL)
            current->type = strDuplicate("data");
        else
            strcpy(current->type, "data");
        current->ARE = ARE_RELOCATABLE;
        current->value = *DC;
        if (type == STRING) {
            token = strtok(NULL, " \t");
            *DC += stringLine(token, isCorrect, lineNum);
        } else {
            while ((token = strtok(NULL, ",")) != NULL)
                (*DC)++;
        }
    } else if (type == INSTRUCTION) {
        opcode = getOpcode(opcodes, token);
        current->type = strDuplicate("code");
        current->value = *IC;
        (*IC) += processInstruction(token, opcodes, opcode, lineNum) + 1;
    } else {
        printError(UNKNOWN_OPERATOR, lineNum);
        *isCorrect = INCORRECT;;
    }
    return labels_last;
}


/*  */
int processInstruction(char *token, opcode_table *opcodes, int opcode, int lineNum){
    int j, addressingMode, (*allowed_modes)[MAX_MODES], isReg = 0, IC = 0;
    char delim[3];

    for (j = 1; token != NULL && j <= opcodes->max_ops[opcode]; j++) { /* 'j' is operand number */
        if (j == 1)
            strcpy(delim, " \t");
        else
            strcpy(delim, ",");
        token = strtok(NULL, delim);
        token = deleteWhiteSpaces(token);
        if (token[0] == ',' && j == 1){
            printError(ILLEGAL_COMMA, lineNum);
            token++;
        }
        if (j > opcodes->max_ops[opcode])
            printError(EXTRANEOUS_TEXT, lineNum);
        if (j == 1)
            isReg = (token[0] == 'r' && isdigit(token[1]));
        addressingMode = getAddressingMode(token);
        if (addressingMode < 0) { /* ERROR */
            printError(addressingMode, lineNum);
            return INCORRECT;
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