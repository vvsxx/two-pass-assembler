#include "header.h"

list *processLabel (SentenceType  type, int *DC, int *IC, char *labelName, char *token, list *labels_last, list *labels_head, opcode_table *opcodes, int lineNum, int *isCorrect);
int stringLine(char *token, int *isCorrect, int lineNum);
int processInstruction(char *token, opcode_table *opcodes, int opcode, int lineNum);


void firstPass(char *fileName, list  *symbols, opcode_table *opcodes){
    list *tmp, *symbols_head = symbols; /* list processing */
    char line[LINE_LENGTH], *buffer, *token; /* line processing */
    char *newFileName;
    char labelName[LABEL_LENGTH];
    int IC = 100, DC = 0, lineNum = 0; /* counters */
    int isCorrect = 1; /* errors flag */
    int *pIC = &IC, *pDC = &DC, *p_isCorrect = &isCorrect; /* pointers */
    int opcode; /* opcode decimal value */
    SentenceType type;
    FILE *input;
    newFileName = safeMalloc(sizeof (fileName) + sizeof (".am")); /* +.am */
    strcpy(newFileName, fileName);
    strcat(newFileName, ".am");
    input = openFile(newFileName, "r");
    buffer = safeMalloc(sizeof (line));
    /* read each line from .am file */
    while (fgets(line, sizeof(line), input) != NULL){
        memset(labelName, '\0', strlen(labelName));
        lineNum++;
        memset(buffer, '\0', strlen(buffer));
        strcpy(buffer,line);
        line[strcspn(line, "\n")] = '\0';
        token = strtok(line, " ,\t");
        type = getSentence(opcodes, token);
        if (type == LABEL){ /* label declaration case */
            strcpy(labelName, token);
            if ((token = strtok(NULL, " \t")) == NULL) /* label before empty line */
                continue;
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
        } else { /* Illegal case */
            printError(UNKNOWN_OPERATOR,lineNum);
            isCorrect = 0;
        }
    }
    /* add up counters */
    tmp = symbols_head;
    while (tmp != NULL){
        if (strcmp(tmp->type, "data") == 0)
            tmp->value += IC;
        tmp = tmp->next;
    }
    isCorrect = createEntries(symbols_head, newFileName);
    free(buffer);
    fclose(input);
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


int createEntries(list *labels, char *fileName){
    int isCorrect = 1;
    list *head;
    char ent_fileName[strlen(fileName)+1], ext_fileName[strlen(fileName)+1];
    FILE *ent = NULL, *ext = NULL;
    strncpy(ent_fileName, fileName, strlen(fileName) - 2);
    strncpy(ext_fileName, fileName, strlen(fileName) - 2);
    strcpy(strrchr(ent_fileName, '.'), ".ent");
    strcpy(strrchr(ext_fileName, '.'), ".ext");
    head = labels;
    while (head != NULL){
        if (head->isExternal) {
            if (ext == NULL)
                ext = openFile(ext_fileName, "w");
            fprintf(ext, "%s\t%.4d\n",head->name,head->value);
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
    free(fileName);
    return isCorrect;
}

list *processLabel (SentenceType type, int *DC, int *IC, char *labelName, char *token, list *labels_last, list *labels_head, opcode_table *opcodes, int lineNum, int *isCorrect){
    int opcode;
    list *current, *tmp;
    labelName[strlen(labelName)-1] = '\0';
    if ((tmp = getElementByName(labels_head, labelName)) != NULL) {
        if (strcmp(tmp->type, "entry") != 0) {
            printError(MULTIPLE_LABEL, lineNum);
            (*isCorrect) = 0;
        } else {
            current = tmp;
        }
    } else {
        labels_last = createSymbol(labels_last, labelName, labelName, type);
        current = labels_last;
    }

    if (type == DATA || type == STRING){
        current->type = strDuplicate("data");
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
        *isCorrect = 0;
    }
    return labels_last;
}


/*  */
int processInstruction(char *token, opcode_table *opcodes, int opcode, int lineNum){
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